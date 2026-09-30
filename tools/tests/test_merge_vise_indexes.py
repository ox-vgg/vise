"""Format-level tests for the isolated VISE generation merger."""

import csv
import tempfile
import unittest
from pathlib import Path

from merge_vise_indexes import (
    IndexEntry,
    ProtoDbReader,
    ProtoDbWriter,
    combine_word_postings,
    merge,
    posting_docids,
    read_dataset,
    remap_posting,
    shift_posting,
    write_dataset,
)


def _entry(ids, *, diff=True, data=None):
    entry = IndexEntry()
    if diff:
        entry.diffid.extend([ids[0], *(b - a for a, b in zip(ids, ids[1:]))])
    else:
        entry.id.extend(ids)
    entry.qx.extend(range(len(ids)))
    entry.qy.extend(range(len(ids)))
    entry.qel_scale = bytes([1] * len(ids))
    entry.qel_ratio = bytes([2] * len(ids))
    entry.qel_angle = bytes([3] * len(ids))
    entry.data = data if data is not None else b"\x01\x02\x03\x04" * len(ids)
    return entry.SerializeToString()


def _project(root: Path, names, postings, *, vocab=b"same vocab", bits=32):
    data = root / "data"
    data.mkdir(parents=True)
    rows = [(name, 400, 300) for name in names]
    write_dataset(data / "index_dset.bin", rows)
    writer = ProtoDbWriter(data / "index_fidx.bin", "index")
    for doc in range(len(names)):
        writer.add_chunk(doc, _entry([0], diff=False))
    writer.close(len(names))
    writer = ProtoDbWriter(data / "index_iidx.bin", "index")
    for word, chunks in enumerate(postings):
        for chunk in chunks:
            writer.add_chunk(word, chunk)
    writer.close(len(postings))
    (data / "filelist.txt").write_text("\n".join(names) + "\n", encoding="utf-8")
    with (data / "filestat.txt").open("w", encoding="utf-8", newline="") as f:
        csv_writer = csv.writer(f)
        csv_writer.writerow(["# src_filename", "dst_filename", "src_width", "src_height", "dst_width", "dst_height"])
        for name in names:
            csv_writer.writerow([name, name, 400, 300, 400, 300])
    (data / "conf.txt").write_text(
        "project_name=fixture\nsearch_engine=relja_retrival\n"
        f"hamm_embedding_bits={bits}\nresize_dimension=400x400\n"
        "sift_scale_3=true\nuse_root_sift=true\nthis-file-saved-by=VISE-2.0.1\n",
        encoding="utf-8",
    )
    (data / "index_status.txt").write_text("start,filelist,traindesc,cluster,assign,hamm,index,end\n")
    (data / "bowcluster.bin").write_bytes(vocab)
    (data / "trainhamm.bin").write_bytes(b"same hamming")
    return root


class MergeTests(unittest.TestCase):
    def test_trailing_featureless_source_documents_are_retained_and_normalized(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg", "b.jpg", "blank.jpg"], [[_entry([0, 1])]])
            forward = ProtoDbWriter(base / "data/index_fidx.bin", "index")
            forward.add_chunk(0, _entry([0], diff=False))
            forward.add_chunk(1, _entry([0], diff=False))
            forward.close(2)  # VISE omits the trailing featureless docID 2.
            result = merge(base, None, root / "out", False)
            self.assertEqual(result["merged_count"], 3)
            self.assertEqual(result["weights"]["num_docs"], 3)
            with ProtoDbReader(root / "out/data/index_fidx.bin") as db:
                self.assertEqual(db.num_ids, 3)
                self.assertEqual(list(db.chunks(2)), [])

    def test_reader_accepts_native_empty_proto_db(self):
        from merge_vise_indexes import END_MARK, Header
        import struct
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "empty.bin"
            header = Header()
            header.offset.append(0)
            raw = header.SerializeToString()
            path.write_bytes(raw + struct.pack("<II", len(raw), END_MARK))
            with ProtoDbReader(path) as db:
                self.assertEqual(db.num_ids, 0)

    def test_rejects_app_directory_override_before_creating_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg"], [[_entry([0])]])
            conf = base / "data/conf.txt"
            conf.write_text(conf.read_text() + "app_dir=/other/application\n")
            with self.assertRaisesRegex(ValueError, "directory overrides"):
                merge(base, None, root / "out", False)
            self.assertFalse((root / "out").exists())

    def test_delta_can_have_a_different_highest_observed_visual_word(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg"], [[_entry([0])]])
            delta = _project(root / "delta", ["b.jpg"], [[], [_entry([0])]])
            merge(base, delta, root / "out", False)
            with ProtoDbReader(root / "out/data/index_iidx.bin") as db:
                self.assertEqual(db.num_ids, 2)
                self.assertEqual(posting_docids(IndexEntry.FromString(next(db.chunks(1)))), [1])

    def test_64_bit_descriptors_survive_delete_replace_add(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg", "b.jpg"],
                            [[_entry([0, 1], data=b"A" * 8 + b"B" * 8)]], bits=64)
            delta = _project(root / "delta", ["b.jpg", "c.jpg"],
                             [[_entry([0, 1], data=b"X" * 8 + b"C" * 8)]], bits=64)
            result = merge(base, delta, root / "out", False, ("b.jpg",))
            self.assertEqual(result["hamming_bits"], 64)
            with ProtoDbReader(root / "out/data/index_iidx.bin") as db:
                entry = IndexEntry.FromString(next(db.chunks(0)))
            self.assertEqual(entry.data, b"A" * 8 + b"X" * 8 + b"C" * 8)
            self.assertEqual(posting_docids(entry), [0, 1, 2])

    def test_full_filenames_with_the_same_stem_are_distinct(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["one/a.jpg", "two/a.jpg"], [[_entry([0, 1])]])
            delta = _project(root / "delta", ["one/a.jpeg"], [[_entry([0])]])
            merge(base, delta, root / "out", False, ("one/a.jpg",))
            self.assertEqual([row[0] for row in read_dataset(root / "out/data/index_dset.bin")],
                             ["two/a.jpg", "one/a.jpeg"])

    def test_deletion_only_does_not_require_a_delta_or_source_images(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg", "b.jpg"], [[_entry([0, 1])]])
            before = {path.name: path.read_bytes() for path in (base / "data").iterdir()}
            report = merge(base, None, root / "out", False, ("a.jpg",))
            self.assertEqual(report["delta_count"], 0)
            self.assertEqual(report["merged_count"], 1)
            self.assertEqual(before, {path.name: path.read_bytes() for path in (base / "data").iterdir()})
            with self.assertRaisesRegex(ValueError, "empty VISE"):
                merge(base, None, root / "empty", False, ("a.jpg", "b.jpg"))
            self.assertFalse((root / "empty").exists())

    def test_materializes_nested_source_mapping_without_hardlinks(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["one/a.jpg"], [[_entry([0])]])
            for directory in ("image_src", "image"):
                (base / directory / "one").mkdir(parents=True)
                (base / directory / "one/a.jpg").write_bytes(b"old-image")
            merge(base, None, root / "out")
            (base / "image/one/a.jpg").write_bytes(b"changed")
            self.assertEqual((root / "out/image/one/a.jpg").read_bytes(), b"old-image")

    def test_bad_descriptor_width_leaves_failed_output_and_preserves_source(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["a.jpg"], [[_entry([0], data=b"bad")]])
            before = (base / "data/index_iidx.bin").read_bytes()
            with self.assertRaisesRegex(ValueError, "bit width"):
                merge(base, None, root / "out", False)
            self.assertTrue((root / "out/MERGE_FAILED.txt").exists())
            self.assertFalse((root / "out/data/index_status.txt").exists())
            self.assertEqual((base / "data/index_iidx.bin").read_bytes(), before)

    def test_rejects_unsafe_filename_and_redirected_config_before_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base = _project(root / "base", ["../a.jpg"], [[_entry([0])]])
            with self.assertRaisesRegex(ValueError, "portable relative"):
                merge(base, None, root / "out", False)
            self.assertFalse((root / "out").exists())
            conf = base / "data/conf.txt"
            conf.write_text(conf.read_text() + "image_dir=/elsewhere\n")
            with self.assertRaisesRegex(ValueError, "overrides"):
                merge(base, None, root / "out", False)

    def test_shift_changes_only_posting_document_ids(self):
        original = _entry([0, 0, 2], data=b"three compressed descriptors")
        shifted = shift_posting(original, 566, 3)
        old, new = IndexEntry.FromString(original), IndexEntry.FromString(shifted)
        self.assertEqual(posting_docids(new), [566, 566, 568])
        self.assertEqual(new.data, old.data)
        self.assertEqual(list(new.qx), list(old.qx))
        self.assertEqual(new.qel_scale, old.qel_scale)
        self.assertEqual(posting_docids(IndexEntry.FromString(shift_posting(_entry([1, 2], diff=False), 566, 3))), [567, 568])

    def test_merge_coalesces_word_chunks_and_offsets_forward_index(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            base = _project(root / "base", ["a.jpg", "b.jpg"], [[_entry([0], data=b"AAAA")], [_entry([1])]])
            delta = _project(root / "delta", ["c.jpg", "d.jpg"], [[_entry([0, 1], data=b"CCCCDDDD")], [_entry([1])]])
            merged = root / "merged"
            result = merge(base, delta, merged, materialize_images=False)
            self.assertEqual(result["merged_count"], 4)
            self.assertEqual(list((merged / "image_src").iterdir()), [])
            self.assertEqual(list((merged / "image").iterdir()), [])
            self.assertTrue((merged / "data/weight.bin").is_file())
            self.assertTrue((merged / "data/index_status.txt").read_bytes().endswith(b",end"))
            self.assertEqual([r[0] for r in read_dataset(merged / "data/index_dset.bin")], ["a.jpg", "b.jpg", "c.jpg", "d.jpg"])
            with ProtoDbReader(merged / "data/index_iidx.bin") as db:
                self.assertEqual(db.num_ids, 2)
                entries = [IndexEntry.FromString(chunk) for chunk in db.chunks(0)]
                self.assertEqual([posting_docids(entry) for entry in entries], [[0, 2, 3]])
                self.assertEqual([entry.data for entry in entries], [b"AAAACCCCDDDD"])
                self.assertEqual([posting_docids(IndexEntry.FromString(chunk)) for chunk in db.chunks(1)], [[1, 3]])
            with ProtoDbReader(merged / "data/index_fidx.bin") as db:
                self.assertEqual(db.num_ids, 4)
                self.assertTrue(list(db.chunks(3)))

    def test_dataset_rechunks_across_1000_boundary(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "dataset.bin"
            rows = [(f"f{i:04}.jpg", 400, 300) for i in range(1001)]
            write_dataset(path, rows)
            with ProtoDbReader(path) as db:
                self.assertEqual(db.num_ids, 2)
            self.assertEqual(read_dataset(path), rows)

    def test_rejects_mismatched_vocabulary_and_existing_output(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            base = _project(root / "base", ["a.jpg"], [[_entry([0])]])
            delta = _project(root / "delta", ["b.jpg"], [[_entry([0])]], vocab=b"different")
            with self.assertRaisesRegex(ValueError, "visual vocabulary"):
                merge(base, delta, root / "merged", materialize_images=False)
            self.assertFalse((root / "merged").exists())
            (root / "merged").mkdir()
            with self.assertRaisesRegex(FileExistsError, "overwrite"):
                merge(base, delta, root / "merged", materialize_images=False)

    def test_rejects_out_of_range_posting(self):
        with self.assertRaisesRegex(ValueError, "out-of-range"):
            shift_posting(_entry([2]), 566, 2)

    def test_filter_and_remap_posting_keeps_feature_data_aligned(self):
        original = _entry([0, 1, 1, 2], data=b"AAAABBBBCCCCDDDD")
        mapped = remap_posting(original, [None, 0, 1])
        entry = IndexEntry.FromString(mapped)
        self.assertEqual(posting_docids(entry), [0, 0, 1])
        self.assertEqual(list(entry.qx), [1, 2, 3])
        self.assertEqual(entry.data, b"BBBBCCCCDDDD")
        self.assertEqual(entry.qel_scale, b"\x01" * 3)

    def test_delete_replace_and_add_in_one_generation(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            base = _project(root / "base", ["a.jpg", "b.jpg", "c.jpg"], [[_entry([0, 1, 2], data=b"AAAABBBBCCCC")]])
            delta = _project(root / "delta", ["b.jpg", "d.jpg"], [[_entry([0, 1], data=b"XXXXYYYY")]])
            result = merge(base, delta, root / "merged", materialize_images=False, remove_filenames=("a.jpg", "b.jpg"))
            self.assertEqual(result["merged_count"], 3)
            self.assertEqual(result["pure_delete_count"], 1)
            self.assertEqual(result["replacement_count"], 1)
            self.assertEqual(result["new_addition_count"], 1)
            self.assertEqual([row[0] for row in read_dataset(root / "merged/data/index_dset.bin")], ["c.jpg", "b.jpg", "d.jpg"])
            with ProtoDbReader(root / "merged/data/index_iidx.bin") as db:
                chunks = list(db.chunks(0))
                self.assertEqual(len(chunks), 1)
                posting = IndexEntry.FromString(chunks[0])
                self.assertEqual(posting_docids(posting), [0, 1, 2])
                self.assertEqual(posting.data, b"CCCCXXXXYYYY")
            with ProtoDbReader(root / "merged/data/index_fidx.bin") as db:
                self.assertEqual(db.num_ids, 3)

    def test_replacement_requires_explicit_base_removal(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            base = _project(root / "base", ["a.jpg"], [[_entry([0])]])
            delta = _project(root / "delta", ["a.jpg"], [[_entry([0])]])
            with self.assertRaisesRegex(ValueError, "without replacement removal"):
                merge(base, delta, root / "merged", materialize_images=False)
            with self.assertRaisesRegex(ValueError, "missing from base"):
                merge(base, delta, root / "merged", materialize_images=False, remove_filenames=("missing.jpg",))

    def test_rejects_unsupported_or_missing_vise_index_format_version(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            base = _project(root / "base", ["a.jpg"], [[_entry([0])]])
            delta = _project(root / "delta", ["b.jpg"], [[_entry([0])]])
            conf = base / "data/conf.txt"
            conf.write_text(conf.read_text().replace("VISE-2.0.1", "VISE-9.9.9"))
            with self.assertRaisesRegex(ValueError, "unsupported VISE index format"):
                merge(base, delta, root / "bad-base", materialize_images=False)
            conf.write_text(conf.read_text().replace("VISE-9.9.9", "VISE-2.0.1"))
            delta_conf = delta / "data/conf.txt"
            delta_conf.write_text(delta_conf.read_text().replace("this-file-saved-by=VISE-2.0.1\n", ""))
            with self.assertRaisesRegex(ValueError, "unsupported VISE index format"):
                merge(base, delta, root / "bad-delta", materialize_images=False)

    def test_coalescing_keeps_descriptor_alignment(self):
        combined, count = combine_word_postings(
            [(_entry([0, 0], data=b"abcdefgh"), 0), (_entry([0, 1], data=b"ijklmnop"), 2)],
            base_count=2,
            delta_count=2,
        )
        entry = IndexEntry.FromString(combined)
        self.assertEqual(count, 4)
        self.assertEqual(posting_docids(entry), [0, 0, 2, 3])
        self.assertEqual(entry.data, b"abcdefghijklmnop")
        self.assertEqual(len(entry.qx), 4)
        self.assertEqual(len(entry.qel_scale), 4)


if __name__ == "__main__":
    unittest.main()
