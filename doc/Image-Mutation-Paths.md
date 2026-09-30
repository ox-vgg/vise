# Image upload and deletion paths

In the normal project manager, `PUT /PROJECT/FILENAME` writes into the loaded
project's configured `image_src_dir`; `DELETE /PROJECT/FILENAME` removes a
regular file from its configured `image_dir`. These remain gallery file
operations, not index updates. Existing serve-only restrictions still apply.

The project must exist and be loaded by the manager. A missing project returns
404; an existing unloaded or failed project returns 412. Uploads are allowed
before an index is built. No operation loads a project implicitly.

Filenames are assembled from the parsed URI components after the namespace and
query have been removed, then percent-decoded exactly once. Nested relative
paths and encoded spaces/separators are supported. Invalid percent escapes,
empty components, dot/parent components, absolute paths, backslashes, colons
(including Windows alternate data streams), NUL and control characters return
400. Windows additionally rejects trailing-dot/space aliases and reserved DOS
device names.

The destination store and each parent directory must already exist. The
canonical parent must remain inside that store. Uploads do not create parent
directories. Both operations reject symlinks, including links to another file
inside the store and dangling links, and reject directory targets. An unavailable
store, invalid target type or failed file operation returns 412. A successful
write or deletion returns 200; the actual output stream result is checked.

Configuration remains trusted: administrators can explicitly configure stores
outside the project's default layout. This check is not an authorization
mechanism or protection against local writers racing filesystem checks, and
stores should not contain hardlinks to files outside the store. Coordinate
local store changes with the server; no platform-specific directory-handle
locking is introduced here.

`test_image_mutation_paths` runs native request dispatch in a temporary project
store and exercises binary uploads, overwrites, normal and nested deletion,
encoded names, namespace/query isolation, missing/unloaded projects, malformed
and escaping names, directory rejection and symlink targets. Checks remain
active in Release. The fixture requires permission to create native symlinks.
