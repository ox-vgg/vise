"""Refresh lifecycle tests with synthetic native-index fixtures, never a live server."""

import io
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from PIL import Image

import refresh_vise_generation as refresh
from test_merge_vise_indexes import _entry, _project


class RefreshTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / "source"
        self.source.mkdir()
        self.generations = self.root / "generations"
        self.template = _project(self.root / "template", ["template.jpg"], [[_entry([0])]])
        self.cli = self.root / "dummy-cli"
        self.cli.write_bytes(b"test-only executable placeholder")
        self.args = SimpleNamespace(source_root=self.source, generation_root=self.generations,
                                    template_project=self.template, manifest=None, apply=True,
                                    vise_cli=self.cli, www=self.root, verify_command=None,
                                    verify_timeout=30, index_timeout=30, index_only=True, min_free_bytes=0)

    def image(self, filename, color):
        path = self.source / filename
        path.parent.mkdir(parents=True, exist_ok=True)
        buffer = io.BytesIO()
        Image.new("RGB", (16, 12), color).save(buffer, "PNG")
        path.write_bytes(buffer.getvalue())

    def fake_build(self, project, entries, source_root, template, cli, timeout):
        _project(project, [row["indexed_filename"] for row in entries],
                 [[_entry(list(range(len(entries))))]])
        for directory in ("image", "image_src", "tmp"):
            (project / directory).mkdir()
        for row in entries:
            raw = refresh.image_bytes(source_root, row["path"])
            if refresh.sha256(raw) != row["sha256"]:
                raise ValueError("changed during build")
            for directory in ("image", "image_src"):
                (project / directory / row["indexed_filename"]).write_bytes(refresh.jpeg_bytes(raw))
        from compute_vise_weight import write_weight
        write_weight(project)

    def fake_verify(self, cli, www, project, query, filename, timeout=300):
        names = (project / "data/filelist.txt").read_text().splitlines()
        self.assertEqual(list((project / "image").iterdir()), [])
        self.assertTrue(query.is_file())
        return {"result": "passed", "project": project.name, "cold_loads": 2,
                "filelist": {"FLIST_FILENAME": names}, "matches": [{"filename": filename}]}

    def apply(self):
        with patch.object(refresh, "build_delta", side_effect=self.fake_build), \
                patch("verify_vise_generation.verify", side_effect=self.fake_verify):
            return refresh.run_once(self.args)

    def test_read_only_plan_does_not_create_generation_root(self):
        self.image("a.png", "red")
        self.args.apply = False
        result = refresh.run_once(self.args)
        self.assertEqual(result["additions"], ["a.png"])
        self.assertFalse(self.generations.exists())

    def test_query_selection_ignores_trailing_featureless_native_documents(self):
        from merge_vise_indexes import ProtoDbWriter
        project = _project(self.root / "trailing", ["a.jpg", "blank.jpg"], [[_entry([0])]])
        writer = ProtoDbWriter(project / "data/index_fidx.bin", "index")
        writer.add_chunk(0, _entry([0], diff=False))
        writer.close(1)
        rows = [{"id": "a", "indexed_filename": "a.jpg"},
                {"id": "blank", "indexed_filename": "blank.jpg"}]
        self.assertEqual(refresh.query_entries(project, rows, set()), [rows[0]])

    def test_arbitrary_manifest_ids_and_duplicate_stems_get_distinct_names(self):
        self.image("one/a.png", "red")
        self.image("two/a.png", "blue")
        manifest = self.root / "source.json"
        manifest.write_text(json.dumps({"version": 1, "images": [
            {"id": "customer / artwork alpha", "path": "one/a.png"},
            {"id": "customer / artwork beta", "path": "two/a.png"}]}))
        rows = refresh.snapshot(self.source, manifest)
        self.assertEqual(len({row["indexed_filename"] for row in rows}), 2)
        self.assertTrue(all(row["indexed_filename"].endswith(".jpg") for row in rows))

    def test_bootstrap_publishes_only_after_image_free_verification(self):
        self.image("a.png", "red")
        result = self.apply()
        self.assertEqual(result["result"], "promoted")
        project, rows, _ = refresh.active_state(self.generations, self.generations / "active.json")
        self.assertEqual([row["id"] for row in rows], ["a.png"])
        self.assertEqual(list((project / "image_src").iterdir()), [])
        self.assertTrue((project / "verification.json").is_file())

    def test_add_replace_delete_builds_only_changed_images_and_preserves_predecessor(self):
        self.image("a.png", "red")
        self.image("b.png", "blue")
        first = self.apply()
        old = self.generations / first["project_name"]
        old_manifest = (old / "gallery-manifest.json").read_bytes()
        self.image("a.png", "green")
        (self.source / "b.png").unlink()
        self.image("c.png", "yellow")
        with patch.object(refresh, "build_delta", side_effect=self.fake_build) as build, \
                patch("verify_vise_generation.verify", side_effect=self.fake_verify):
            result = refresh.run_once(self.args)
        self.assertEqual({row["id"] for row in build.call_args.args[1]}, {"a.png", "c.png"})
        self.assertEqual(result["replacements"], ["a.png"])
        self.assertEqual(result["removals"], ["b.png"])
        self.assertEqual((old / "gallery-manifest.json").read_bytes(), old_manifest)
        self.assertTrue((old / "data/index_iidx.bin").exists())

    def test_deletion_only_preserves_features_without_running_indexer(self):
        self.image("a.png", "red")
        self.image("b.png", "blue")
        self.apply()
        (self.source / "b.png").unlink()
        with patch.object(refresh, "build_delta") as build, \
                patch("verify_vise_generation.verify", side_effect=self.fake_verify):
            result = refresh.run_once(self.args)
        build.assert_not_called()
        self.assertEqual(result["images"], 1)
        self.assertEqual(result["removals"], ["b.png"])

    def test_unchanged_poll_never_builds_or_verifies_a_generation(self):
        self.image("a.png", "red")
        self.apply()
        with patch.object(refresh, "build_delta") as build, patch.object(refresh, "verify_generation") as verify:
            self.assertEqual(refresh.run_once(self.args)["result"], "up_to_date")
        build.assert_not_called()
        verify.assert_not_called()

    def test_failed_native_self_query_keeps_the_old_pointer(self):
        self.image("a.png", "red")
        self.apply()
        pointer = self.generations / "active.json"
        before = pointer.read_bytes()
        self.image("b.png", "blue")
        with patch.object(refresh, "build_delta", side_effect=self.fake_build), \
                patch("verify_vise_generation.verify", side_effect=RuntimeError("cold-load search failed")):
            with self.assertRaisesRegex(RuntimeError, "cold-load search failed"):
                refresh.run_once(self.args)
        self.assertEqual(pointer.read_bytes(), before)
        self.assertTrue(list(self.generations.glob("*/REFRESH_FAILED.txt")))

    def test_native_zero_exit_without_completed_index_does_not_publish(self):
        self.image("a.png", "red")
        with patch.object(refresh.subprocess, "run", return_value=SimpleNamespace(returncode=0)):
            with self.assertRaisesRegex(ValueError, "native delta build failed"):
                refresh.run_once(self.args)
        self.assertFalse((self.generations / "active.json").exists())
        self.assertTrue(list(self.generations.glob("*/REFRESH_FAILED.txt")))

    def test_source_change_after_snapshot_is_rejected_before_native_build(self):
        self.image("a.png", "red")
        self.apply()
        pointer = self.generations / "active.json"
        original = pointer.read_bytes()
        self.image("b.png", "blue")
        original_snapshot = refresh.snapshot
        def changed_snapshot(*args):
            rows = original_snapshot(*args)
            self.image("b.png", "green")
            return rows
        with patch.object(refresh, "snapshot", side_effect=changed_snapshot), \
                patch.object(refresh.subprocess, "run") as native:
            with self.assertRaisesRegex(ValueError, "changed after the frozen"):
                refresh.run_once(self.args)
        native.assert_not_called()
        self.assertEqual(pointer.read_bytes(), original)

    def test_verifier_zero_status_with_wrong_filelist_never_publishes(self):
        self.image("a.png", "red")
        with patch.object(refresh, "build_delta", side_effect=self.fake_build), \
                patch("verify_vise_generation.verify", return_value={"result": "passed", "cold_loads": 2}):
            with self.assertRaisesRegex(ValueError, "did not prove"):
                refresh.run_once(self.args)
        self.assertFalse((self.generations / "active.json").exists())

    def test_pointer_compare_and_swap_preserves_an_external_change(self):
        self.image("a.png", "red")
        self.apply()
        pointer = self.generations / "active.json"
        original = pointer.read_bytes()
        self.image("b.png", "blue")
        def race(*args):
            pointer.write_bytes(original + b" ")
        with patch.object(refresh, "build_delta", side_effect=self.fake_build), \
                patch.object(refresh, "verify_generation", side_effect=race):
            with self.assertRaisesRegex(ValueError, "pointer changed"):
                refresh.run_once(self.args)
        self.assertEqual(pointer.read_bytes(), original + b" ")

    def test_empty_source_never_replaces_a_valid_index(self):
        self.image("a.png", "red")
        self.apply()
        pointer = self.generations / "active.json"
        original = pointer.read_bytes()
        (self.source / "a.png").unlink()
        with self.assertRaisesRegex(ValueError, "empty VISE"):
            refresh.run_once(self.args)
        self.assertEqual(pointer.read_bytes(), original)

    def test_invalid_manifest_hash_and_escaping_path_are_rejected(self):
        self.image("a.png", "red")
        manifest = self.root / "source.json"
        manifest.write_text(json.dumps({"version": 1, "images": [{"id": "a", "path": "a.png", "sha256": "bad"}]}))
        with self.assertRaisesRegex(ValueError, "SHA-256"):
            refresh.snapshot(self.source, manifest)
        manifest.write_text(json.dumps({"version": 1, "images": [{"id": "a", "path": "../outside.png"}]}))
        with self.assertRaisesRegex(ValueError, "relative path"):
            refresh.snapshot(self.source, manifest)

    def test_refresh_lock_prevents_a_second_writer(self):
        self.generations.mkdir()
        (self.generations / ".refresh.lock").write_text("held")
        with self.assertRaises(FileExistsError):
            refresh.run_once(self.args)
        self.assertEqual((self.generations / ".refresh.lock").read_text(), "held")


if __name__ == "__main__":
    unittest.main()
