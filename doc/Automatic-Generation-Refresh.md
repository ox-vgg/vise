# Optional source-directory and manifest refresh

`tools/refresh_vise_generation.py` is an optional example for applications that
add, replace, or remove images after their first index build. It depends on the
[offline index merge tools](Offline-Index-Updates.md). It scans a local image
directory or consumes a JSON manifest, builds only changed images using a
separate `vise-cli --cmd=create-project` process, and publishes a new generation
through `active.json`. It does not update a running VISE server's project map;
the consuming application must read the pointer and route new queries to the
selected project. Earlier generations are retained for application rollback.

Install the optional Python dependencies:

```sh
python -m pip install -r tools/refresh-requirements.txt
```

Start with a read-only plan. The generation root need not exist yet:

```sh
python tools/refresh_vise_generation.py --source-root /gallery \
  --generation-root /vise-generations --template-project /projects/template
```

For the first build, `--template-project` supplies a VISE 2.0.1 configuration
and compatible `bowcluster.bin`/`trainhamm.bin` files. It is read-only. The
initial generation indexes all source entries; later cycles reuse the active
generation's features and vocabulary. Directory mode uses complete relative
paths as external identities. Manifest mode allows arbitrary stable string IDs
and changing source paths:

```json
{
  "version": 1,
  "images": [
    {"id": "artwork alpha", "path": "collection-a/photo.png"},
    {"id": "artwork beta", "path": "collection-b/photo.jpg"}
  ]
}
```

Pass the file with `--manifest /catalog/images.json`; paths are relative to
`--source-root`. An optional `sha256` field requires the source bytes to match
that digest. Source file content is hashed on every cycle; changes to content
or path for an existing identity are replacements. Original IDs are stored in
`gallery-manifest.json` and mapped to SHA-256-derived JPEG filenames for native
VISE indexing. This avoids native preprocessing collisions between names such
as `a.jpg` and `a.png`, preserves arbitrary IDs without business-specific UUID
rules, and avoids exposing arbitrary IDs in native project filenames.

Build, verify, and publish explicitly:

```sh
python tools/refresh_vise_generation.py --source-root /gallery \
  --generation-root /vise-generations --template-project /projects/template \
  --vise-cli /opt/vise/vise-cli --www /opt/vise/www --apply --index-only
```

`--index-only` leaves the published generation's `image/` and `image_src/`
directories empty. The changed images still exist temporarily in the newly
built delta. Retained images are not downloaded or re-extracted for indexing;
one retained image and one changed image, when searchable, are read solely for
verification. Each native verifier starts a new process bound to a random
loopback port, checks the full paginated filelist, extracts and searches the
query image, requires its own filename in the results, then repeats from a
second cold process and checks the same features/ranking. This validates the
project after its image files have been removed. Run later cycles with the same
`--index-only` choice; an index-only predecessor cannot supply gallery images
for a later materialized-image merge.

Without `--index-only`, independent gallery image copies are kept. Native
metadata, visual groups, and application URLs are not migrated. Original source
files and old generations are never deleted. A deletion-only update does not
run an indexer. If no searchable images remain, publication is refused.

`--poll-seconds 120` repeats the process. A shared exclusive `.refresh.lock`
serializes this tool's writers. A frozen source snapshot drives each cycle;
changed bytes detected while staging or reading a smoke query abort that cycle.
New source entries arriving during a build are picked up on the next cycle.
Native failures, omitted images, incorrect filelists, failed self-queries, low
disk space, and a changed active pointer leave the predecessor selected.
Unpublished outputs receive `REFRESH_FAILED.txt`; native verification failures
also preserve `native-verification-error.log`. Inspect failures before retrying.
The tool does not remove stale locks automatically.

The pointer contains the project name, SHA-256 of its manifest, and predecessor
name. Publication compares the old pointer bytes and writes the new file using
`os.replace` after flushing it. All writers must use the same lock; an unrelated
writer that ignores this protocol can still race a filesystem compare/replace.
The consumer should verify the manifest digest and pin one generation for the
duration of a query. Cleanup and live-server routing/leases belong to the
application, so this example performs neither.

For a custom native verification environment, `--verify-command` accepts a JSON
argv list; it is executed directly without a shell. Available placeholders are
`{project}`, `{query}`, `{filename}`, and `{report}`. The command must finish
within `--verify-timeout`, return zero, and write a JSON report to `{report}`
with `result="passed"`, the exact `project` name, `cold_loads >= 2`, a complete
`filelist.FLIST_FILENAME` array, and `matches` containing the queried
`filename`. A successful process exit alone never authorizes publication.

The built-in verifier can also be used independently:

```sh
python tools/verify_vise_generation.py --vise-cli /opt/vise/vise-cli \
  --www /opt/vise/www --project /vise-generations/SELECTED_GENERATION \
  --query /tmp/query.jpg --expected-filename INDEXED_FILENAME.jpg \
  --output /tmp/native-verification.json
```

This is a batch-update example, not an in-place concurrent indexing API. It
inherits the offline merge tool's format restrictions and rewrites the full
resulting index. Every poll reads and hashes source files, so large datasets
may need a catalog/change-feed adapter in the consuming application. The
default free-space floor is 1 GiB and can be raised with `--min-free-bytes`;
it is a floor before/after construction, not a peak-size prediction. Images
are bounded to 100 MiB and 80 million decoded pixels; oversized or unreadable
files abort publication. Symlinks/junctions, escaped source paths, empty output
indexes, and unsupported formats/configurations are rejected. The source and
generation trees must not overlap. Run against quiescent source/template
projects, with trusted executable paths. This example depends on the
[offline merge tools (PR #9)](https://github.com/ox-vgg/vise/pull/9). Small delta
construction needs the [first-observed-word fix (PR #13)](https://github.com/ox-vgg/vise/pull/13);
native queries containing words absent from the collection need the
[absent-visual-word retrieval fix (PR #14)](https://github.com/ox-vgg/vise/pull/14).
Use a VISE build containing these fixes and the
[protoDb ID-boundary fix (PR #10)](https://github.com/ox-vgg/vise/pull/10) before
enabling unattended refresh. The example refuses publication when native
construction or verification fails; it does not apply runtime patches itself.
`--verify-timeout` is an overall deadline for each two-process self-query check,
including startup, all filelist pages, extraction, and both searches.

Windows builds need the optional `vise-cli` target enabled; see
[Windows build support (PR #12)](https://github.com/ox-vgg/vise/pull/12). This tool starts
only its own temporary index/verification processes and hides their console
windows. It never restarts services or installs a scheduler.
