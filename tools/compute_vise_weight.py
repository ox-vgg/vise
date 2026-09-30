"""Precompute VISE 2.0.1 tfidfData before loading a fresh index generation.

This follows tfidf_v2.cpp for the observed hard-assignment index format:
one indexEntry per word, diffid postings, with no count/weight fields.
This avoids a Windows reader race between background inverted-index loading
and foreground IDF computation. The pinned VISE version is required.
"""

import argparse
import json
import math
import os
import struct
import time
from pathlib import Path

from google.protobuf import descriptor_pb2, descriptor_pool, message_factory

from merge_vise_indexes import IndexEntry, ProtoDbReader, plain_file, validate_conf, validate_posting


F32 = struct.Struct("<f")


def tfidf_class():
    file = descriptor_pb2.FileDescriptorProto()
    file.name = "vise_tfidf_data.proto"
    file.package = "rr"
    file.syntax = "proto2"
    message = file.message_type.add()
    message.name = "tfidfData"
    for name, number in (("idf", 1), ("docL2", 2)):
        field = message.field.add()
        field.name = name
        field.number = number
        field.type = descriptor_pb2.FieldDescriptorProto.TYPE_FLOAT
        field.label = descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED
        field.options.packed = True
    pool = descriptor_pool.DescriptorPool()
    pool.Add(file)
    return message_factory.GetMessageClass(pool.FindMessageTypeByName("rr.tfidfData"))


TfidfData = tfidf_class()


def f32(number: float) -> float:
    return F32.unpack(F32.pack(number))[0]


def compute(project: Path) -> tuple[bytes, dict]:
    started = time.perf_counter()
    conf = validate_conf(project)
    with ProtoDbReader(plain_file(project / "data/index_fidx.bin")) as forward:
        num_docs = forward.num_ids
    doc_sq = [0.0] * num_docs
    idfs = []
    feature_count = 0
    with ProtoDbReader(plain_file(project / "data/index_iidx.bin")) as inverted:
        num_words = inverted.num_ids
        for word in range(num_words):
            chunks = list(inverted.chunks(word))
            if not chunks:
                idfs.append(f32(math.log(num_docs)))
                continue
            if len(chunks) != 1:
                raise ValueError(f"expected one entry at word {word}, got {len(chunks)}")
            entry = IndexEntry.FromString(chunks[0])
            validate_posting(entry, num_docs, int(conf["hamm_embedding_bits"]))
            if entry.id or entry.count or entry.weight or not entry.diffid:
                raise ValueError(f"unexpected posting encoding at word {word}")
            unique = 1 + sum(delta != 0 for delta in entry.diffid[1:])
            idf_double = math.log(num_docs / max(1, unique))
            idf_float = f32(idf_double)
            idfs.append(idf_float)
            doc = 0
            previous_doc = None
            count = 0
            for delta in entry.diffid:
                doc += delta
                if doc >= num_docs:
                    raise ValueError(f"out-of-range doc ID {doc} in word {word}")
                if previous_doc is not None and doc != previous_doc:
                    doc_sq[previous_doc] += (count * idf_float) ** 2
                    count = 0
                previous_doc = doc
                count += 1
                feature_count += 1
            if previous_doc is not None:
                doc_sq[previous_doc] += (count * idf_float) ** 2
    norms = [f32(math.sqrt(value)) if value > 1e-7 else 1.0 for value in doc_sq]
    result = TfidfData()
    result.idf.extend(idfs)
    result.docL2.extend(norms)
    report = {
        "project": str(project),
        "num_docs": num_docs,
        "num_words": num_words,
        "num_features": feature_count,
        "zero_norm_docs": sum(value <= 1e-7 for value in doc_sq),
        "compute_seconds": round(time.perf_counter() - started, 3),
        "weight_bytes": len(result.SerializeToString()),
    }
    return result.SerializeToString(), report


def write_weight(project: Path) -> dict:
    """Compute/verify before native cold load; never replace a different weight."""
    data, report = compute(project)
    destination = project / "data/weight.bin"
    if destination.exists():
        if plain_file(destination).read_bytes() != data:
            raise ValueError("existing VISE weight differs from the expected IDF and norms")
        report["action"] = "verified_existing"
    else:
        temporary = destination.with_suffix(".bin.part")
        with temporary.open("xb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, destination)
        report["action"] = "written"
    (project / "weight-report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--compare-existing", type=Path)
    args = parser.parse_args()
    if args.output.exists() or args.report.exists():
        raise FileExistsError("refusing to overwrite weight or report")
    data, report = compute(args.project)
    if args.compare_existing:
        actual = TfidfData.FromString(args.compare_existing.read_bytes())
        predicted = TfidfData.FromString(data)
        if len(actual.idf) != len(predicted.idf) or len(actual.docL2) != len(predicted.docL2):
            raise ValueError("existing TF-IDF dimensions differ from the computed index")
        report["existing_bytes_identical"] = data == args.compare_existing.read_bytes()
        report["idf_exact_count"] = sum(a == b for a, b in zip(actual.idf, predicted.idf))
        report["doc_l2_exact_count"] = sum(a == b for a, b in zip(actual.docL2, predicted.docL2))
        report["max_idf_abs_diff"] = max(abs(a - b) for a, b in zip(actual.idf, predicted.idf))
        report["max_doc_l2_abs_diff"] = max(abs(a - b) for a, b in zip(actual.docL2, predicted.docL2))
    args.output.write_bytes(data)
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
