"""Small index fixtures for VISE 2.0.1's offline IDF and norm writer."""

import math
import tempfile
import unittest
from pathlib import Path

from compute_vise_weight import TfidfData, compute, f32, write_weight
from merge_vise_indexes import IndexEntry, ProtoDbWriter



class WeightTests(unittest.TestCase):
    def make_project(self, project: Path) -> None:
        data = project / "data"
        data.mkdir(parents=True)
        (data / "conf.txt").write_text("this-file-saved-by=VISE-2.0.1\nsearch_engine=relja_retrival\nhamm_embedding_bits=32\n")
        forward = ProtoDbWriter(data / "index_fidx.bin", "index")
        for doc in range(3):
            forward.add_chunk(doc, IndexEntry().SerializeToString())
        forward.close(4)
        inverted = ProtoDbWriter(data / "index_iidx.bin", "index")
        first = IndexEntry()
        first.data = b"\x00" * 12
        first.diffid.extend([0, 0, 1])  # doc 0 twice, doc 1 once
        second = IndexEntry()
        second.data = b"\x00" * 8
        second.diffid.extend([1, 1])  # doc 1, doc 2
        inverted.add_chunk(0, first.SerializeToString())
        inverted.add_chunk(1, second.SerializeToString())
        inverted.close(2)

    def test_matches_expected_idf_and_doc_norms_with_featureless_image(self):
        with tempfile.TemporaryDirectory() as temporary:
            project = Path(temporary) / "project"
            self.make_project(project)
            raw, report = compute(project)
            weight = TfidfData.FromString(raw)
            idf = f32(math.log(2))
            self.assertEqual(list(weight.idf), [idf, idf])
            self.assertEqual(list(weight.docL2), [
                f32(2 * idf), f32(math.sqrt(2 * idf * idf)), idf, 1.0,
            ])
            self.assertEqual(report["num_features"], 5)
            self.assertEqual(report["zero_norm_docs"], 1)

    def test_refuses_existing_weight_with_different_values(self):
        with tempfile.TemporaryDirectory() as temporary:
            project = Path(temporary) / "project"
            self.make_project(project)
            report = write_weight(project)
            self.assertEqual(report["action"], "written")
            self.assertEqual(write_weight(project)["action"], "verified_existing")
            (project / "data/weight.bin").write_bytes(b"wrong")
            with self.assertRaisesRegex(ValueError, "differs"):
                write_weight(project)


if __name__ == "__main__":
    unittest.main()
