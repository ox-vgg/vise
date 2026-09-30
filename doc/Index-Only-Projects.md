# Serving an index with an external gallery

An application that already stores images elsewhere can serve a completed VISE
index without retaining a second local gallery. This is an explicit serving mode:
add `index_only=true` to the project's `data/conf.txt` before loading the project.
The default (key absent, or `index_only=false`) keeps normal VISE gallery behavior.
Only the exact values `true` and `false` are accepted.

Build and verify the index with normal VISE first. Keep a separate build project
or an independently verified backup of the source images for future indexing.
Enable the option in a **copy** of the completed project and verify external
queries there before removing any local images. VISE does not delete images or
automatically convert projects when this option changes.

In this mode `image/`, `image_src/`, `image_small/` and `tmp/` may be absent; VISE
does not recreate them on load. The `data/` directory and all the files required
to load the completed index must remain available. Loading may still append to
`data/index.log` or compute `data/weight.bin`; this option does not make the
entire index directory immutable or provide authentication.

Use these existing native interfaces with an external gallery:

- `GET /PROJECT/index_status?response_format=json`
- `GET /PROJECT/file_feature_status?file_id=N`, when the optional per-file
  diagnostic API is available; this mode preserves its normal handling
- `GET /PROJECT/filelist?response_format=json` (including its usual pagination)
- `GET /PROJECT/_conf`
- `POST /PROJECT/_extract_image_features` with a query image
- `POST /PROJECT/_search_using_features?response_format=json` with those features
- `POST /PROJECT/_get_feature_match_details` with features and its usual match parameters

The returned file IDs and filenames still identify the indexed snapshot. Your
application resolves them to its own images and enforces access control and
version consistency. VISE does not fetch external URLs, generate thumbnails, or
verify that remote image bytes still match the indexed snapshot. The existing
query/feature upload limits continue to apply.

Local `image`, `image_src`, `image_small` and `_cover_image` routes return 404.
The project home explains the mode. Built-in gallery HTML pages (including the
HTML filelist, app assets and external search UI), image registration and local gallery
mutations return 412 rather than a page with broken thumbnails or an attempt to
read absent pixels. JSON filelist and index status remain available. Project
index load/unload remain available; rebuilding the index, file uploads, file
additions, configuration changes and registration are disabled while the mode
is active. Application-level project administration is unchanged.

An index-only project requires an existing completed index; it cannot be used
to create a new index. To rebuild, use the separate gallery/build project and
publish another completed snapshot. An empty local source count of zero does
not mean the index is empty: inspect the JSON filelist for indexed files.

## Integration regression

The optional `test_index_only` target copies only the `data/` tree of a disposable
completed test project into unique temporary projects. It verifies default-mode
directory creation, two cold loads without image directories, query feature-byte
and full ranked-result parity, JSON endpoints, missing-image behavior, rejected
mutations, incomplete-index rejection and invalid option values. It resets the
VLFeat random seed for each comparison because each vocabulary forest otherwise
uses a different randomized approximate-search tree. It never alters the input
project. Use a query resized to the fixture's indexing dimensions.

Configure with `-DVISE_BUILD_INDEX_ONLY_TEST=ON`, build `test_index_only`, then run:

```sh
test_index_only /path/to/disposable/completed/project /path/to/query.jpg
```

Set `VISE_INDEX_ONLY_TEST_PROJECT` and `VISE_INDEX_ONLY_TEST_QUERY` at configuration
time to register the same test with CTest. Use a small public or synthetic image
fixture whose query returns results; production gallery data is unnecessary.
