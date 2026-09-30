"""Offline, copy-on-write merger for two VISE 2.0.1 index generations.

Reads completed source projects and writes a new project; never changes a source.
Supports hard-assignment indexes with matching visual vocabularies and 32/64-bit
Hamming descriptors. Filenames (including directories/extensions), not stems,
are document identities. See doc/Offline-Index-Updates.md for limitations.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import shutil
import struct
from pathlib import Path, PurePosixPath
from typing import Iterator

from google.protobuf import descriptor_pb2, descriptor_pool, message_factory


END_MARK = 0x0FD0FD02
U32 = struct.Struct("<I")
DATASET_BATCH = 1000
MAX_U32 = 0xFFFFFFFF
SUPPORTED_VISE_SAVE_VERSION = "VISE-2.0.1"
INDEX_KEYS = ("search_engine", "hamm_embedding_bits", "resize_dimension", "sift_scale_3", "use_root_sift")
PATH_KEYS = ("data_dir", "image_dir", "image_src_dir", "tmp_dir", "image_small_dir", "app_dir")


def _message_classes():
    fd = descriptor_pb2.FileDescriptorProto()
    fd.name = "vise_merge_index.proto"
    fd.package = "rr"
    fd.syntax = "proto2"

    def add_message(name, fields):
        message = fd.message_type.add()
        message.name = name
        for name_, number, type_, repeated, packed in fields:
            field = message.field.add()
            field.name = name_
            field.number = number
            field.type = type_
            field.label = (
                descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED
                if repeated
                else descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
            )
            if packed:
                field.options.packed = True

    T = descriptor_pb2.FieldDescriptorProto
    add_message("protoDbHeader", [
        ("offset", 1, T.TYPE_UINT64, True, True),
        ("description", 2, T.TYPE_STRING, False, False),
    ])
    add_message("datasetEntry", [
        ("filename", 1, T.TYPE_STRING, True, False),
        ("width", 2, T.TYPE_UINT32, True, True),
        ("height", 3, T.TYPE_UINT32, True, True),
    ])
    add_message("indexEntry", [
        ("id", 1, T.TYPE_UINT32, True, True),
        ("diffid", 2, T.TYPE_UINT32, True, True),
        ("x", 3, T.TYPE_FLOAT, True, True),
        ("y", 4, T.TYPE_FLOAT, True, True),
        ("a", 5, T.TYPE_FLOAT, True, True),
        ("b", 6, T.TYPE_FLOAT, True, True),
        ("c", 7, T.TYPE_FLOAT, True, True),
        ("qx", 8, T.TYPE_UINT32, True, True),
        ("qy", 9, T.TYPE_UINT32, True, True),
        ("qel_scale", 10, T.TYPE_BYTES, False, False),
        ("qel_ratio", 11, T.TYPE_BYTES, False, False),
        ("qel_angle", 12, T.TYPE_BYTES, False, False),
        ("count", 13, T.TYPE_UINT32, True, True),
        ("weight", 14, T.TYPE_FLOAT, True, True),
        ("data", 15, T.TYPE_BYTES, False, False),
        ("keep", 16, T.TYPE_BOOL, True, True),
        ("docid", 51, T.TYPE_UINT32, True, True),
    ])
    pool = descriptor_pool.DescriptorPool()
    pool.Add(fd)
    return (
        message_factory.GetMessageClass(pool.FindMessageTypeByName("rr.protoDbHeader")),
        message_factory.GetMessageClass(pool.FindMessageTypeByName("rr.datasetEntry")),
        message_factory.GetMessageClass(pool.FindMessageTypeByName("rr.indexEntry")),
    )


Header, DatasetEntry, IndexEntry = _message_classes()


class ProtoDbReader:
    def __init__(self, path: Path):
        self.path = Path(path)
        self.file = self.path.open("rb")
        size = self.file.seek(0, os.SEEK_END)
        if size < 8:
            raise ValueError(f"truncated protoDb: {path}")
        self.file.seek(size - 8)
        header_size, marker = struct.unpack("<II", self.file.read(8))
        if marker != END_MARK or header_size > size - 8:
            raise ValueError(f"invalid protoDb trailer: {path}")
        header_start = size - 8 - header_size
        self.file.seek(header_start)
        self.header = Header.FromString(self.file.read(header_size))
        self.offsets = tuple(self.header.offset)
        if (
            len(self.offsets) < 1
            or self.offsets[0] != 0
            or self.offsets[-1] != header_start
            or any(a > b for a, b in zip(self.offsets, self.offsets[1:]))
        ):
            raise ValueError(f"invalid protoDb offsets: {path}")
        self.num_ids = len(self.offsets) - 1

    def close(self):
        self.file.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def chunks(self, identifier: int) -> Iterator[bytes]:
        if identifier < 0 or identifier >= self.num_ids:
            raise IndexError(identifier)
        pos, end = self.offsets[identifier : identifier + 2]
        self.file.seek(pos)
        while pos < end:
            if end - pos < 4:
                raise ValueError(f"truncated chunk size at {identifier}: {self.path}")
            length_bytes = self.file.read(4)
            if len(length_bytes) != 4:
                raise ValueError(f"truncated chunk size at {identifier}: {self.path}")
            length = U32.unpack(length_bytes)[0]
            pos += 4 + length
            if pos > end:
                raise ValueError(f"chunk exceeds ID boundary at {identifier}: {self.path}")
            chunk = self.file.read(length)
            if len(chunk) != length:
                raise ValueError(f"truncated chunk at {identifier}: {self.path}")
            yield chunk


class ProtoDbWriter:
    def __init__(self, path: Path, description: str):
        self.path = Path(path)
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.file = self.path.open("wb")
        self.offsets: list[int] = []
        self.description = description
        self.last_id = -1

    def add_chunk(self, identifier: int, chunk: bytes):
        if identifier < self.last_id:
            raise ValueError("protoDb IDs must be non-descending")
        while len(self.offsets) <= identifier:
            self.offsets.append(self.file.tell())
        self.file.write(U32.pack(len(chunk)))
        self.file.write(chunk)
        self.last_id = identifier

    def close(self, num_ids: int):
        if num_ids <= self.last_id or num_ids < 1:
            raise ValueError("invalid final protoDb ID count")
        while len(self.offsets) < num_ids:
            self.offsets.append(self.file.tell())
        self.offsets.append(self.file.tell())
        header = Header()
        header.offset.extend(self.offsets)
        header.description = self.description
        data = header.SerializeToString()
        self.file.write(data)
        self.file.write(struct.pack("<II", len(data), END_MARK))
        self.file.close()


def read_dataset(path: Path) -> list[tuple[str, int, int]]:
    rows = []
    with ProtoDbReader(path) as db:
        for identifier in range(db.num_ids):
            chunks = list(db.chunks(identifier))
            if len(chunks) != 1:
                raise ValueError(f"dataset ID {identifier} has {len(chunks)} chunks")
            entry = DatasetEntry.FromString(chunks[0])
            if len(entry.filename) != len(entry.width) or len(entry.filename) != len(entry.height):
                raise ValueError(f"dataset dimensions mismatch at ID {identifier}")
            if not entry.filename or (identifier < db.num_ids - 1 and len(entry.filename) != DATASET_BATCH):
                raise ValueError(f"dataset batch size mismatch at ID {identifier}")
            rows.extend(zip(entry.filename, entry.width, entry.height))
    return rows


def write_dataset(path: Path, rows: list[tuple[str, int, int]]):
    if not rows:
        raise ValueError("empty dataset")
    writer = ProtoDbWriter(path, "dataset")
    for start in range(0, len(rows), DATASET_BATCH):
        entry = DatasetEntry()
        for filename, width, height in rows[start : start + DATASET_BATCH]:
            entry.filename.append(filename)
            entry.width.append(width)
            entry.height.append(height)
        writer.add_chunk(start // DATASET_BATCH, entry.SerializeToString())
    writer.close((len(rows) + DATASET_BATCH - 1) // DATASET_BATCH)


def posting_docids(entry: IndexEntry) -> list[int]:
    if entry.diffid and entry.id:
        raise ValueError("posting contains both id and diffid")
    if entry.diffid:
        values = []
        total = 0
        for delta in entry.diffid:
            total += delta
            values.append(total)
        return values
    return list(entry.id)


def shift_posting(chunk: bytes, offset: int, delta_count: int) -> bytes:
    entry = IndexEntry.FromString(chunk)
    docs = posting_docids(entry)
    if any(doc >= delta_count for doc in docs):
        raise ValueError("delta posting refers to an out-of-range document")
    if offset + delta_count > MAX_U32:
        raise ValueError("merged doc IDs exceed uint32")
    if entry.docid:
        raise ValueError("unexpected indexing-stage docid in final inverted index")
    if entry.diffid:
        entry.diffid[0] += offset
    elif entry.id:
        for index, doc in enumerate(entry.id):
            entry.id[index] = doc + offset
    return entry.SerializeToString()


def remap_posting(chunk: bytes, old_to_new: list[int | None]) -> bytes | None:
    """Filter deleted features and remap retained doc IDs in one posting chunk."""
    entry = IndexEntry.FromString(chunk)
    docs = posting_docids(entry)
    if not docs or any(doc >= len(old_to_new) for doc in docs):
        raise ValueError("posting refers to an out-of-range document or is empty")
    if entry.docid:
        raise ValueError("unexpected indexing-stage docid in final inverted index")
    selected = [i for i, old_doc in enumerate(docs) if old_to_new[old_doc] is not None]
    if not selected:
        return None
    output = IndexEntry()
    mapped = [old_to_new[docs[i]] for i in selected]
    assert all(doc is not None for doc in mapped)
    if any(a > b for a, b in zip(mapped, mapped[1:])):
        raise ValueError("posting remap must preserve document order")
    output.diffid.extend([mapped[0], *(b - a for a, b in zip(mapped, mapped[1:]))])
    n = len(docs)
    for field in ("x", "y", "a", "b", "c", "qx", "qy", "count", "weight", "keep"):
        values = getattr(entry, field)
        if len(values) not in (0, n):
            raise ValueError(f"posting field {field} has invalid length")
        if values:
            getattr(output, field).extend(values[i] for i in selected)
    for field in ("qel_scale", "qel_ratio", "qel_angle", "data"):
        data = getattr(entry, field)
        if data and len(data) % n:
            raise ValueError(f"posting bytes {field} are not feature-aligned")
        width = len(data) // n
        setattr(output, field, b"".join(data[i * width : (i + 1) * width] for i in selected))
    return output.SerializeToString()


def combine_word_postings(chunks: list[tuple[bytes, int]], base_count: int, delta_count: int) -> tuple[bytes, int]:
    """Produce one native VISE indexEntry for a word from ordered source chunks.

    VISE 2.0.1's indexEntryVector cannot safely traverse multiple entries for
    one word in the tested binary. Thus raw protoDb chunk concatenation is not
    sufficient even though the container format permits it.
    """
    merged = IndexEntry()
    previous = -1
    feature_count = 0
    shape = None
    for raw, offset in chunks:
        entry = IndexEntry.FromString(raw)
        docs = posting_docids(entry)
        max_count = base_count if offset == 0 else delta_count
        if not docs or any(doc >= max_count for doc in docs):
            raise ValueError("posting refers to an out-of-range document or is empty")
        shifted = [doc + offset for doc in docs]
        if shifted[0] < previous or any(a > b for a, b in zip(shifted, shifted[1:])):
            raise ValueError("postings are not globally ordered")
        previous = shifted[-1]
        n = len(docs)
        fields = ("x", "y", "a", "b", "c", "qx", "qy", "count", "weight", "keep")
        byte_fields = ("qel_scale", "qel_ratio", "qel_angle", "data")
        this_shape = tuple(bool(getattr(entry, field)) for field in (*fields, *byte_fields))
        if shape is not None and shape != this_shape:
            raise ValueError("incompatible posting fields within one word")
        shape = this_shape
        for field in fields:
            values = getattr(entry, field)
            if len(values) not in (0, n):
                raise ValueError(f"posting field {field} has invalid length")
            getattr(merged, field).extend(values)
        for field in byte_fields:
            data = getattr(entry, field)
            if data and len(data) % n:
                raise ValueError(f"posting bytes {field} are not feature-aligned")
            setattr(merged, field, getattr(merged, field) + data)
        merged.id.extend(shifted)
        feature_count += n
    if not feature_count:
        raise ValueError("cannot merge empty posting list")
    ids = list(merged.id)
    merged.ClearField("id")
    merged.diffid.extend([ids[0], *(b - a for a, b in zip(ids, ids[1:]))])
    data = merged.SerializeToString()
    if len(data) > 50_000_000:
        raise ValueError("merged posting exceeds VISE's 50 MB entry limit")
    return data, feature_count


def _sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(4 * 1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def _conf(path: Path) -> dict[str, str]:
    return dict(
        line.strip().split("=", 1)
        for line in path.read_text(encoding="utf-8").splitlines()
        if line.strip() and "=" in line
    )


def safe_filename(name: str) -> str:
    """Use VISE's portable relative filename as the complete document key."""
    if (not isinstance(name, str) or not name or "\\" in name or ":" in name
            or any(ord(char) < 32 for char in name)):
        raise ValueError("filename must be a portable relative path")
    parts = name.split("/")
    if any(part in ("", ".", "..") for part in parts) or PurePosixPath(name).is_absolute():
        raise ValueError("filename must be a portable relative path")
    return name


def plain_file(path: Path) -> Path:
    """Reject redirected paths before reading or copying a project file."""
    for item in (path, *path.parents):
        if item.is_symlink() or (hasattr(item, "is_junction") and item.is_junction()):
            raise ValueError(f"redirected project path: {path}")
    if not path.is_file():
        raise ValueError(f"missing project file: {path}")
    return path


def validate_conf(project: Path) -> dict[str, str]:
    conf = _conf(plain_file(project / "data/conf.txt"))
    if conf.get("this-file-saved-by") != SUPPORTED_VISE_SAVE_VERSION:
        raise ValueError("unsupported VISE index format: " + conf.get("this-file-saved-by", "<missing>"))
    if conf.get("search_engine") != "relja_retrival":
        raise ValueError("only the relja_retrival search engine is supported")
    if conf.get("hamm_embedding_bits") not in ("32", "64"):
        raise ValueError("only 32-bit and 64-bit Hamming encodings are supported")
    if any(key in conf for key in PATH_KEYS):
        raise ValueError("project directory overrides are unsupported; use the standard project layout")
    return conf


def validate_posting(entry: IndexEntry, num_docs: int, bits: int) -> None:
    docs = posting_docids(entry)
    if not docs or any(doc >= num_docs for doc in docs) or any(a > b for a, b in zip(docs, docs[1:])):
        raise ValueError("posting document IDs are empty, out-of-range, or unordered")
    if entry.docid or entry.count or entry.weight or entry.keep:
        raise ValueError("only completed hard-assignment postings without count/weight/keep are supported")
    known = IndexEntry()
    known.CopyFrom(entry)
    known.DiscardUnknownFields()
    if known.SerializeToString() != entry.SerializeToString():
        raise ValueError("unknown indexEntry fields are unsupported")
    for field in ("x", "y", "a", "b", "c", "qx", "qy"):
        if len(getattr(entry, field)) not in (0, len(docs)):
            raise ValueError(f"posting field {field} is not feature-aligned")
    for field in ("qel_scale", "qel_ratio", "qel_angle"):
        if len(getattr(entry, field)) not in (0, len(docs)):
            raise ValueError(f"posting field {field} is not feature-aligned")
    if len(entry.data) != len(docs) * (bits // 8):
        raise ValueError("Hamming descriptor bytes do not match the configured bit width")


def _filestat_rows(path: Path) -> list[list[str]]:
    with path.open(encoding="utf-8-sig", newline="") as stream:
        reader = csv.reader(stream)
        header = next(reader)
        if header != ["# src_filename", "dst_filename", "src_width", "src_height", "dst_width", "dst_height"]:
            raise ValueError(f"unknown filestat header: {path}")
        return list(reader)


def _project_rows(project: Path):
    conf = validate_conf(project)
    if (plain_file(project / "data/index_status.txt").read_text().strip().split(",")[-1] != "end"):
        raise ValueError("source indexing incomplete")
    rows = read_dataset(plain_file(project / "data/index_dset.bin"))
    names = [safe_filename(row[0]) for row in rows]
    if len(names) != len(set(names)):
        raise ValueError("duplicate filenames within a source generation")
    if plain_file(project / "data/filelist.txt").read_text(encoding="utf-8-sig").splitlines() != names:
        raise ValueError("filelist/dataset order mismatch")
    with ProtoDbReader(plain_file(project / "data/index_fidx.bin")) as forward:
        # Native builders omit trailing featureless documents altogether.
        # Their dataset rows are still valid; absent forward IDs mean empty.
        if forward.num_ids > len(rows):
            raise ValueError("forward index exceeds the dataset document count")
    stats = _filestat_rows(plain_file(project / "data/filestat.txt"))
    # Preprocessing can complete in an order different from lexical filename order.
    by_name = {}
    for stat in stats:
        if len(stat) != 6:
            raise ValueError("invalid filestat row")
        safe_filename(stat[0])
        safe_filename(stat[1])
        if stat[1] in by_name:
            raise ValueError("duplicate destination in filestat")
        by_name[stat[1]] = stat
    if set(by_name) != set(names):
        raise ValueError("filestat/filelist mismatch (resize_dimension=-1 is unsupported)")
    return conf, rows, [by_name[name] for name in names]


def _copy_images(sources, output: Path):
    seen = {"image": set(), "image_src": set()}
    for source, stats in sources:
        for row in stats:
            for directory, name in (("image_src", row[0]), ("image", row[1])):
                if name in seen[directory]:
                    raise ValueError(f"duplicate {directory} filename: {name}")
                seen[directory].add(name)
                src = plain_file(source / directory / name)
                dst = output / directory / name
                dst.parent.mkdir(parents=True, exist_ok=True)
                # A new generation owns its bytes, including when sources later change.
                shutil.copy2(src, dst)


def merge(base: Path, delta: Path | None, output: Path, materialize_images: bool = True,
          remove_filenames: tuple[str, ...] = ()) -> dict:
    """Merge compatible indexes; delta=None permits a deletion-only generation."""
    base, output = Path(base), Path(output)
    delta = Path(delta) if delta is not None else None
    sources = [base] + ([delta] if delta is not None else [])
    if output.exists():
        raise FileExistsError(f"refusing to overwrite existing generation: {output}")
    for source in sources:
        if not source.is_dir():
            raise ValueError("source project must exist")
        if output.resolve().is_relative_to(source.resolve()):
            raise ValueError("output must be outside the source projects")
    base_conf, base_rows, base_stat = _project_rows(base)
    if delta is None:
        delta_rows, delta_stat = [], []
    else:
        delta_conf, delta_rows, delta_stat = _project_rows(delta)
        for key in INDEX_KEYS:
            if base_conf.get(key) != delta_conf.get(key):
                raise ValueError(f"incompatible index setting: {key}")
    bits = int(base_conf["hamm_embedding_bits"])
    vocab_hashes = {}
    for name in ("bowcluster.bin", "trainhamm.bin"):
        digest = _sha256(plain_file(base / "data" / name))
        if delta is not None and digest != _sha256(plain_file(delta / "data" / name)):
            raise ValueError(f"incompatible visual vocabulary: {name}")
        vocab_hashes[name] = digest
    base_names, delta_names = ([row[0] for row in rows] for rows in (base_rows, delta_rows))
    removed = {safe_filename(name) for name in remove_filenames}
    if len(removed) != len(remove_filenames):
        raise ValueError("duplicate removal filename")
    if removed - set(base_names):
        raise ValueError("removal filenames missing from base")
    if (set(base_names) & set(delta_names)) - removed:
        raise ValueError("delta reuses an active base filename without replacement removal")
    kept = [i for i, name in enumerate(base_names) if name not in removed]
    output_rows = [base_rows[i] for i in kept] + delta_rows
    if not output_rows:
        raise ValueError("cannot create an empty VISE index")
    if len(output_rows) > MAX_U32:
        raise ValueError("merged doc IDs exceed uint32")
    base_map = [None] * len(base_rows)
    for new_id, old_id in enumerate(kept):
        base_map[old_id] = new_id
    mappings = [(base, base_map)]
    if delta is not None:
        mappings.append((delta, [len(kept) + i for i in range(len(delta_rows))]))
    output.mkdir(parents=True)
    data = output / "data"
    data.mkdir()
    writer = None
    try:
        write_dataset(data / "index_dset.bin", output_rows)
        writer = ProtoDbWriter(data / "index_fidx.bin", "index")
        for source, mapping in mappings:
            with ProtoDbReader(plain_file(source / "data/index_fidx.bin")) as forward:
                for old_id, new_id in enumerate(mapping):
                    if new_id is None or old_id >= forward.num_ids:
                        continue
                    for chunk in forward.chunks(old_id):
                        entry = IndexEntry.FromString(chunk)
                        if entry.docid:
                            raise ValueError("unexpected indexing-stage docid in forward index")
                        writer.add_chunk(new_id, chunk)
        writer.close(len(output_rows))
        from contextlib import ExitStack
        with ExitStack() as stack:
            inverted = [stack.enter_context(ProtoDbReader(plain_file(source / "data/index_iidx.bin")))
                        for source, _ in mappings]
            # protoDb stores IDs only through the highest observed word, which
            # can differ between tiny deltas sharing exactly the same vocabulary.
            words = max(db.num_ids for db in inverted)
            writer = ProtoDbWriter(data / "index_iidx.bin", "index")
            word_count = posting_count = 0
            for word in range(words):
                chunks = []
                for db, (_, mapping) in zip(inverted, mappings):
                    if word >= db.num_ids:
                        continue
                    for chunk in db.chunks(word):
                        validate_posting(IndexEntry.FromString(chunk), len(mapping), bits)
                        mapped = remap_posting(chunk, mapping)
                        if mapped is not None:
                            chunks.append((mapped, 0))
                if chunks:
                    combined, count = combine_word_postings(chunks, len(output_rows), len(output_rows))
                    writer.add_chunk(word, combined)
                    word_count += 1
                    posting_count += count
            writer.close(words)
        names = [row[0] for row in output_rows]
        (data / "filelist.txt").write_text("\n".join(names) + "\n", encoding="utf-8")
        with (data / "filestat.txt").open("w", encoding="utf-8", newline="") as stream:
            csv_writer = csv.writer(stream, lineterminator="\n")
            csv_writer.writerow(["# src_filename", "dst_filename", "src_width", "src_height", "dst_width", "dst_height"])
            csv_writer.writerows([base_stat[i] for i in kept] + delta_stat)
        for name in vocab_hashes:
            shutil.copy2(base / "data" / name, data / name)
        base_conf["project_name"] = output.name
        (data / "conf.txt").write_text("\n".join(f"{k}={v}" for k, v in sorted(base_conf.items())) + "\n", encoding="utf-8")
        for directory in ("image_src", "image", "tmp"):
            (output / directory).mkdir()
        if materialize_images:
            image_sources = [(base, [base_stat[i] for i in kept])]
            if delta is not None:
                image_sources.append((delta, delta_stat))
            _copy_images(image_sources, output)
        from compute_vise_weight import write_weight
        weight_report = write_weight(output)
        if read_dataset(data / "index_dset.bin") != output_rows:
            raise ValueError("merged dataset read-back differs from inputs")
        report = {"base_project": str(base), "delta_project": str(delta) if delta else None,
                  "output_project": str(output), "base_count": len(base_rows),
                  "delta_count": len(delta_rows), "merged_count": len(names),
                  "removed_filenames": sorted(removed),
                  "replacement_count": len(removed & set(delta_names)),
                  "pure_delete_count": len(removed - set(delta_names)),
                  "new_addition_count": len(set(delta_names) - set(base_names)),
                  "word_count": word_count, "posting_count": posting_count,
                  "vocabulary_sha256": vocab_hashes, "hamming_bits": bits,
                  "images_materialized": materialize_images, "weights": weight_report}
        (output / "merge-report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        # Only completed, read-back checked projects are eligible for native loading.
        (data / "index_status.txt").write_text("start,filelist,traindesc,cluster,assign,hamm,index,end", encoding="utf-8")
        return report
    except BaseException:
        if writer is not None and not writer.file.closed:
            writer.file.close()
        (output / "MERGE_FAILED.txt").write_text("This generation failed validation and must not be served.\n", encoding="utf-8")
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", type=Path, required=True)
    parser.add_argument("--delta", type=Path, help="completed compatible delta; omit for deletion-only updates")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--skip-images", action="store_true", help="create an index-only project; image UI/matching will be unavailable")
    parser.add_argument("--remove-filename", action="append", default=[], help="complete indexed filename to delete/replace; repeat per file")
    args = parser.parse_args()
    print(json.dumps(merge(args.base, args.delta, args.output, not args.skip_images, tuple(args.remove_filename)), ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
