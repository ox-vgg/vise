# Diagnosing files without indexed visual features

A completed filelist contains all indexed dataset filenames, including files
for which no local features were extracted. Filelist membership alone therefore
does not establish that a file can contribute matches.

`GET /PROJECT/file_feature_status?file_id=N` returns JSON for one file in a
loaded index:

```json
{"file_id":3,"filename":"blank.jpg","indexed_word_count":0,"has_indexed_features":false}
```

The values come from the final forward index, not an indexing-log warning or a
new image decode. `indexed_word_count` counts visual word entries; this forward
index stores unique word IDs, so it is **not** the number of SIFT descriptors.
`has_indexed_features=false` means the forward index has no visual words for
this document. The file remains in the dataset/filelist, and the endpoint does
not delete it, reindex it, or read its source image. Trailing featureless
documents may be outside the forward index's stored ID range while still being
valid dataset IDs; these report zero as well.

A nonzero value indicates indexed words exist. It does not guarantee a positive
match, a particular rank, or successful search with every query; similarity,
feature matching and spatial verification still determine retrieval results.
IDs describe the currently loaded index snapshot and may change after replacing
that snapshot. Use the returned filename to join with an application's gallery.
The filename and count are read together while preventing index load/unload
from replacing or releasing the data used by this diagnostic.

`file_id` must be an unsigned decimal integer in the 32-bit range. Missing,
empty, signed, nondecimal and overflow values return HTTP 400; an ID outside the
dataset returns 404. An unloaded index returns 412. Normal VISE project loading
and serve-only restrictions apply. The response is always JSON on success.

## Native regression

Build `test_file_feature_status` with `-DVISE_BUILD_FEATURE_STATUS_TEST=ON`.
Provide a small completed disposable VISE index containing both a textured
image and a featureless image, such as a plain white JPEG:

```sh
test_file_feature_status /path/to/test/project blank.jpg
```

The test copies only the input `data/` tree into a unique temporary project and
never changes the input index. It verifies native forward-index queries,
featureless-file status and filelist membership, indexed-word presence,
malformed/unknown IDs, concurrent unload and both native/HTTP unloaded-index errors. Checks remain
active in Release builds. Set `VISE_FEATURE_STATUS_TEST_PROJECT` and
`VISE_FEATURE_STATUS_EMPTY_FILENAME` to register it with CTest.
