# Offline delta index updates

`tools/merge_vise_indexes.py` combines a completed base project with a completed
delta project into a **new** VISE 2.0.1 project. Only new or changed images need
feature extraction. Deleted images are removed from both indexes; replacements
explicitly remove the old indexed filename before adding its new features.
The base and delta projects remain unchanged. The utility recomputes TF-IDF
weights before marking the output complete, so a cold load does not need to
compute weights while loading the inverted index.

This is an optional Python tool, independent of the C++ build:

```sh
python -m pip install -r tools/requirements.txt
python tools/merge_vise_indexes.py --base /projects/base --delta /projects/delta \
  --output /projects/generation2 --remove-filename changed/photo.jpg
```

Prepare the delta using the **same** `bowcluster.bin`, `trainhamm.bin`, feature
configuration, and Hamming bit width as the base. The utility verifies vocabulary
file hashes and relevant settings. It supports 32-bit and 64-bit Hamming
descriptors and validates the descriptor byte width. It rejects unsupported save
versions, soft-assignment/count/weight postings, unknown posting fields, unfinished
indexes, filelist/dataset mismatches, and project directory overrides.

The complete indexed relative filename is a document identity: `one/photo.jpg`,
`two/photo.jpg`, and `one/photo.jpeg` are distinct. `--remove-filename` always
means the name in `data/filelist.txt`, which may differ from the source name after
VISE converts a non-JPEG image. VISE preprocessing may map `a.png` and `a.jpg`
to the same destination; such duplicate indexes are rejected. Portable filenames
must not contain absolute paths, `..`, backslashes, colons, or control characters.

For deletion-only changes, omit `--delta`; retained features are copied without
extracting any image again:

```sh
python tools/merge_vise_indexes.py --base /projects/base \
  --output /projects/generation3 --remove-filename obsolete/photo.jpg
```

By default, resized and source images are copied using the mapping in
`filestat.txt`, including subdirectories. Copies have independent bytes rather
than hard links. `--skip-images` creates empty `image/` and `image_src/`
directories for a search-only deployment with externally stored gallery images.
External image feature extraction and search work from the index; image display,
internal image matching, metadata, and visual-group databases are not migrated by
this utility. Verify the new project in a separate native process before changing
an application's active project. A failed output is retained with
`MERGE_FAILED.txt` for inspection and must not be served.

The output rewrites the complete dataset, forward index, and inverted index;
runtime/disk work is proportional to the resulting index, not just the delta.
Peak resources include the source indexes and new generation. One visual-word
posting remains limited to VISE's 50 MB entry size. Empty output indexes and
`resize_dimension=-1` projects without `filestat.txt` are not supported. Source
projects must be quiescent while reading; do not run an indexer against them.
No server, database, storage, or old-generation cleanup is performed.
Native VISE may omit trailing featureless documents from the forward index.
Those dataset rows are preserved and the new forward index is normalized to
include empty entries through the complete dataset count before weights are
computed. Featureless documents therefore contribute to the resulting document
count and receive unit norms; they are not expected to produce search matches.

`tools/compute_vise_weight.py` also exposes weight computation independently:

```sh
python tools/compute_vise_weight.py --project /projects/base \
  --output /tmp/weight.bin --report /tmp/weight-report.json \
  --compare-existing /projects/base/data/weight.bin
```

Its protobuf wire descriptors mirror `proto_db_header.proto`,
`dataset_entry.proto`, `index_entry.proto`, and `tfidf_data.proto` in the C++
source. The format gate is deliberately VISE 2.0.1. TF-IDF follows
`tfidf_v2.cpp`, including float32 weight rounding and the unit norm assigned to
featureless documents. It supports one hard-assignment posting entry per word,
not every historical Relja Retrieval encoding.

Run the format-level tests (synthetic indexes, no server required):

```sh
PYTHONPATH=tools python -B -m unittest discover -s tools/tests -v
# Windows PowerShell:
$env:PYTHONPATH = (Resolve-Path tools).Path
python -B -m unittest discover -s tools/tests -v
```
