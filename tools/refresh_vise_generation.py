"""Optional local-directory/JSON-manifest VISE generation refresh example.

Builds changed images in an isolated vise-cli process and publishes active.json
only after index checks and repeated native cold-load self-queries. No database,
service management, source deletion, or old-generation cleanup is performed.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time
import uuid

from compute_vise_weight import write_weight
from merge_vise_indexes import (
    ProtoDbReader, _project_rows, merge, plain_file,
    safe_filename, validate_conf,
)

IMAGE_SUFFIXES = {".jpg", ".jpeg", ".png", ".tif", ".tiff", ".bmp", ".webp"}
GENERATION_RE = re.compile(r"^visegen_[0-9]{14}_[a-f0-9]{12}(?:_delta)?$")
MAX_IMAGE_BYTES = 100 * 1024 * 1024
MAX_IMAGE_PIXELS = 80_000_000
MAX_MANIFEST_BYTES = 64 * 1024 * 1024


def plain_directory(path: Path):
    for item in (path, *path.parents):
        if item.is_symlink() or (hasattr(item, "is_junction") and item.is_junction()):
            raise ValueError("directory path is redirected")


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def image_bytes(source_root: Path, relative: str) -> bytes:
    path = plain_file(source_root / safe_filename(relative))
    if not path.resolve().is_relative_to(source_root.resolve()):
        raise ValueError("source image escapes its root")
    if not 0 < path.stat().st_size <= MAX_IMAGE_BYTES:
        raise ValueError("source image size is outside the configured budget")
    data = path.read_bytes()
    if not 0 < len(data) <= MAX_IMAGE_BYTES:
        raise ValueError("source image changed size while reading")
    return data


def jpeg_bytes(raw: bytes) -> bytes:
    from PIL import Image, ImageOps
    with Image.open(io.BytesIO(raw)) as image:
        if image.width * image.height > MAX_IMAGE_PIXELS:
            raise ValueError("source image exceeds the decode pixel budget")
        image = ImageOps.exif_transpose(image)
        image.load()
        output = io.BytesIO()
        image.convert("RGB").save(output, format="JPEG", quality=95)
        return output.getvalue()


def snapshot(source_root: Path, manifest: Path | None) -> list[dict]:
    plain_directory(source_root)
    if not source_root.is_dir() or source_root.is_symlink():
        raise ValueError("source root must be a plain directory")
    if manifest is None:
        inputs = []
        for path in sorted(source_root.rglob("*")):
            if path.is_symlink() or (hasattr(path, "is_junction") and path.is_junction()):
                raise ValueError("source directory contains a redirected path")
            if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES:
                relative = path.relative_to(source_root).as_posix()
                inputs.append({"id": relative, "path": relative})
    else:
        plain_file(manifest)
        if manifest.stat().st_size > MAX_MANIFEST_BYTES:
            raise ValueError("source manifest exceeds the size budget")
        value = json.loads(manifest.read_text(encoding="utf-8"))
        if not isinstance(value, dict) or value.get("version") != 1 or not isinstance(value.get("images"), list):
            raise ValueError("source manifest must contain version=1 and an images list")
        inputs = value["images"]
    rows = []
    identities = set()
    filenames = set()
    for entry in inputs:
        if not isinstance(entry, dict) or not isinstance(entry.get("id"), str) or not entry["id"]:
            raise ValueError("every source image needs a nonempty string id")
        if entry["id"] in identities:
            raise ValueError("duplicate source image id")
        relative = safe_filename(entry.get("path"))
        digest = sha256(image_bytes(source_root, relative))
        if "sha256" in entry and entry["sha256"] != digest:
            raise ValueError("source image does not match the manifest SHA-256")
        # Stable identity is independent of source path, extension, and business data.
        filename = sha256(entry["id"].encode("utf-8")) + ".jpg"
        if filename in filenames:
            raise ValueError("source identity hash collision")
        identities.add(entry["id"])
        filenames.add(filename)
        rows.append({"id": entry["id"], "path": relative, "sha256": digest,
                     "indexed_filename": filename})
    return sorted(rows, key=lambda row: row["id"])


def plan_update(current: list[dict], previous: list[dict]) -> dict:
    live = {row["id"]: row for row in current}
    old = {row["id"]: row for row in previous}
    if len(live) != len(current) or len(old) != len(previous):
        raise ValueError("duplicate image identities in a snapshot")
    return {"additions": sorted(live.keys() - old.keys()),
            "replacements": sorted(key for key in live.keys() & old.keys()
                                   if (live[key]["sha256"], live[key]["path"]) !=
                                   (old[key]["sha256"], old[key]["path"])),
            "removals": sorted(old.keys() - live.keys())}


def active_state(root: Path, pointer: Path):
    if not pointer.exists():
        return None
    original = plain_file(pointer).read_bytes()
    value = json.loads(original)
    if not isinstance(value, dict) or value.get("version") != 1:
        raise ValueError("invalid active pointer version")
    name = value.get("project_name")
    if not isinstance(name, str) or not GENERATION_RE.fullmatch(name):
        raise ValueError("invalid active generation name")
    project = root / name
    manifest = plain_file(project / "gallery-manifest.json")
    contents = manifest.read_bytes()
    if len(contents) > MAX_MANIFEST_BYTES or sha256(contents) != value.get("manifest_sha256"):
        raise ValueError("active generation manifest hash mismatch")
    metadata = json.loads(contents)
    rows = metadata.get("images") if isinstance(metadata, dict) and metadata.get("version") == 1 else None
    if not isinstance(rows, list):
        raise ValueError("invalid generation snapshot")
    ids, names = set(), set()
    for row in rows:
        if (not isinstance(row, dict) or not isinstance(row.get("id"), str) or not row["id"]
                or row["id"] in ids or not isinstance(row.get("sha256"), str)
                or not re.fullmatch(r"[a-f0-9]{64}", row["sha256"])):
            raise ValueError("invalid generation snapshot entry")
        safe_filename(row.get("path"))
        expected = sha256(row["id"].encode("utf-8")) + ".jpg"
        if row.get("indexed_filename") != expected or expected in names:
            raise ValueError("generation identity/filename mapping mismatch")
        ids.add(row["id"])
        names.add(expected)
    _, indexed, _ = _project_rows(project)
    if {row[0] for row in indexed} != names:
        raise ValueError("active index and snapshot filenames disagree")
    return project, rows, original


def build_delta(project: Path, entries: list[dict], source_root: Path,
                template: Path, cli: Path, timeout: int):
    conf = validate_conf(template)
    project.mkdir()
    for directory in ("data", "image", "image_src", "tmp"):
        (project / directory).mkdir()
    for entry in entries:
        raw = image_bytes(source_root, entry["path"])
        if sha256(raw) != entry["sha256"]:
            raise ValueError("source image changed after the frozen snapshot")
        (project / "image_src" / entry["indexed_filename"]).write_bytes(jpeg_bytes(raw))
    for name in ("bowcluster.bin", "trainhamm.bin"):
        shutil.copy2(plain_file(template / "data" / name), project / "data" / name)
    conf["project_name"] = project.name
    (project / "data/conf.txt").write_text(
        "\n".join(f"{key}={value}" for key, value in sorted(conf.items())) + "\n", encoding="utf-8")
    command = [str(cli), "--cmd=create-project", project.name + ":" + str(project / "data/conf.txt")]
    with (project / "build.log").open("wb") as log:
        result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                                timeout=timeout, creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
    # VISE's existing CLI can return zero even if index_create reports failure.
    if result.returncode or not (project / "data/index_status.txt").exists():
        raise ValueError("isolated native delta build failed; inspect build.log")
    _, indexed, _ = _project_rows(project)
    if {row[0] for row in indexed} != {entry["indexed_filename"] for entry in entries}:
        raise ValueError("native delta omitted or changed a requested image")
    write_weight(project)


def query_entries(project: Path, rows: list[dict], delta_ids: set[str]) -> list[dict]:
    filenames = (project / "data/filelist.txt").read_text(encoding="utf-8").splitlines()
    with ProtoDbReader(project / "data/index_fidx.bin") as forward:
        searchable = {name for doc, name in enumerate(filenames)
                      if doc < forward.num_ids and list(forward.chunks(doc))}
    old = [row for row in rows if row["id"] not in delta_ids and row["indexed_filename"] in searchable]
    new = [row for row in rows if row["id"] in delta_ids and row["indexed_filename"] in searchable]
    selected = old[:1] + new[:1]
    if not selected:
        raise ValueError("new generation has no searchable smoke-query image")
    return selected


def verify_generation(project: Path, rows: list[dict], delta_ids: set[str], args):
    from verify_vise_generation import verify
    reports = []
    with tempfile.TemporaryDirectory(prefix="vise-refresh-queries-") as temporary:
        for entry in query_entries(project, rows, delta_ids):
            raw = image_bytes(args.source_root, entry["path"])
            if sha256(raw) != entry["sha256"]:
                raise ValueError("smoke-query image changed after the frozen snapshot")
            query = Path(temporary) / entry["indexed_filename"]
            query.write_bytes(jpeg_bytes(raw))
            if args.verify_command:
                report_path = Path(temporary) / "verification.json"
                report_path.unlink(missing_ok=True)
                substitutions = {"project": str(project), "query": str(query),
                                 "filename": entry["indexed_filename"], "report": str(report_path)}
                command = [part.format(**substitutions) for part in args.verify_command]
                subprocess.run(command, check=True, timeout=args.verify_timeout, stdin=subprocess.DEVNULL,
                               creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
                report = json.loads(plain_file(report_path).read_text(encoding="utf-8"))
            else:
                report = verify(args.vise_cli, args.www, project, query, entry["indexed_filename"],
                                timeout=args.verify_timeout)
            expected = {row["indexed_filename"] for row in rows}
            listed = report.get("filelist", {}).get("FLIST_FILENAME", [])
            matched = [hit.get("filename") for hit in report.get("matches", []) if isinstance(hit, dict)]
            if (report.get("result") != "passed" or report.get("project") != project.name
                    or report.get("cold_loads", 0) < 2 or len(listed) != len(expected)
                    or set(listed) != expected or entry["indexed_filename"] not in matched):
                raise ValueError("native verification did not prove the complete generation and its self-query")
            reports.append({"id": entry["id"], "rank": matched.index(entry["indexed_filename"]) + 1,
                            "report": report})
    (project / "verification.json").write_text(json.dumps(reports, indent=2) + "\n", encoding="utf-8")


def atomic_pointer(pointer: Path, expected: bytes | None, value: dict):
    current = plain_file(pointer).read_bytes() if pointer.exists() else None
    if current != expected:
        raise ValueError("active pointer changed during refresh")
    temporary = pointer.with_name("." + pointer.name + "." + uuid.uuid4().hex + ".tmp")
    try:
        with temporary.open("x", encoding="utf-8") as stream:
            json.dump(value, stream, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, pointer)
    finally:
        temporary.unlink(missing_ok=True)


def run_once(args) -> dict:
    root, pointer = args.generation_root, args.generation_root / "active.json"
    plain_directory(root)
    if root.resolve().is_relative_to(args.source_root.resolve()) or args.source_root.resolve().is_relative_to(root.resolve()):
        raise ValueError("source and generation directories must not overlap")
    if not args.apply:
        previous = active_state(root, pointer)
        rows = snapshot(args.source_root, args.manifest)
        return {"result": "would_refresh", "images": len(rows),
                **plan_update(rows, previous[1] if previous else [])}
    plain_file(args.vise_cli)
    root.mkdir(parents=True, exist_ok=True)
    if root.is_symlink() or (hasattr(root, "is_junction") and root.is_junction()):
        raise ValueError("generation root must be a plain directory")
    lock = root / ".refresh.lock"
    descriptor = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    os.close(descriptor)
    project = None
    try:
        previous = active_state(root, pointer)
        rows = snapshot(args.source_root, args.manifest)
        plan = plan_update(rows, previous[1] if previous else [])
        if not rows:
            raise ValueError("cannot publish an empty VISE index")
        if previous and not any(plan.values()):
            return {"result": "up_to_date", "images": len(rows), **plan}
        if shutil.disk_usage(root).free < args.min_free_bytes:
            raise ValueError("disk free space is below the configured floor")
        template = previous[0] if previous else args.template_project
        if template is None:
            raise ValueError("first generation requires --template-project")
        validate_conf(template)
        generation = "visegen_" + time.strftime("%Y%m%d%H%M%S", time.gmtime()) + "_" + uuid.uuid4().hex[:12]
        changed_ids = set(plan["additions"] + plan["replacements"])
        delta = root / (generation + "_delta")
        if changed_ids:
            project = delta
            build_delta(delta, [row for row in rows if row["id"] in changed_ids],
                        args.source_root, template, args.vise_cli, args.index_timeout)
        if previous is None:
            project = delta
        else:
            project = root / generation
            old = {row["id"]: row for row in previous[1]}
            removed = tuple(old[key]["indexed_filename"] for key in plan["removals"] + plan["replacements"])
            merge(previous[0], delta if changed_ids else None, project, not args.index_only, removed)
        # Only newly built generation files are removed; predecessors stay intact.
        if args.index_only:
            for directory in ("image", "image_src"):
                for path in (project / directory).iterdir():
                    if not path.is_file() or path.is_symlink():
                        raise ValueError("unexpected image in the new generation")
                    path.unlink()
        _, indexed, _ = _project_rows(project)
        if {row[0] for row in indexed} != {row["indexed_filename"] for row in rows}:
            raise ValueError("new generation filelist differs from the frozen source snapshot")
        manifest = project / "gallery-manifest.json"
        manifest.write_text(json.dumps({"version": 1, "images": rows}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        verify_generation(project, rows, changed_ids, args)
        if shutil.disk_usage(root).free < args.min_free_bytes:
            raise ValueError("disk free space fell below the floor before publication")
        value = {"version": 1, "project_name": project.name,
                 "manifest_sha256": sha256(manifest.read_bytes()),
                 "predecessor": previous[0].name if previous else None}
        atomic_pointer(pointer, previous[2] if previous else None, value)
        return {"result": "promoted", "project_name": project.name, "images": len(rows), **plan}
    except BaseException:
        if project is not None and project.is_dir():
            (project / "REFRESH_FAILED.txt").write_text("Generation was not published; inspect before retrying.\n", encoding="utf-8")
        raise
    finally:
        lock.unlink()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, help="version=1 JSON images list; otherwise scan image files recursively")
    parser.add_argument("--generation-root", type=Path, required=True)
    parser.add_argument("--template-project", type=Path, help="completed compatible project supplying config/vocabulary for bootstrap")
    parser.add_argument("--vise-cli", type=Path)
    parser.add_argument("--www", type=Path, help="VISE static web assets, required by the built-in isolated verifier")
    parser.add_argument("--verify-command", help="JSON argv list; placeholders: {project}, {query}, {filename}, {report}")
    parser.add_argument("--apply", action="store_true", help="build and verify; default is a read-only plan")
    parser.add_argument("--index-only", action="store_true", help="publish empty gallery image directories for external image storage")
    parser.add_argument("--index-timeout", type=int, default=3600)
    parser.add_argument("--verify-timeout", type=int, default=300)
    parser.add_argument("--min-free-bytes", type=int, default=1024 ** 3)
    parser.add_argument("--poll-seconds", type=int, default=0, help="repeat; zero performs one cycle")
    args = parser.parse_args(argv)
    for key in ("source_root", "generation_root", "manifest", "template_project", "vise_cli", "www"):
        value = getattr(args, key)
        if value is not None:
            setattr(args, key, value.absolute())
    if args.apply and (not args.vise_cli or (not args.www and not args.verify_command)):
        parser.error("--apply requires --vise-cli and either --www or --verify-command")
    if args.index_timeout <= 0 or args.verify_timeout <= 0 or args.min_free_bytes < 0 or args.poll_seconds < 0:
        parser.error("timeouts must be positive; polling and disk floor cannot be negative")
    if args.verify_command:
        args.verify_command = json.loads(args.verify_command)
        if not isinstance(args.verify_command, list) or not args.verify_command or any(not isinstance(part, str) for part in args.verify_command):
            parser.error("--verify-command must be a nonempty JSON string argv list")
    while True:
        # ASCII JSON keeps arbitrary Unicode IDs portable on Windows consoles.
        print(json.dumps(run_once(args), ensure_ascii=True), flush=True)
        if not args.poll_seconds:
            return 0
        time.sleep(args.poll_seconds)


if __name__ == "__main__":
    sys.exit(main())
