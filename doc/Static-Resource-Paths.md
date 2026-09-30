# Static resources stay within their configured store

VISE serves static resources from its global `http-www-dir` and from a project's
`app`, `image`, `image_src` and `image_small` stores. A project without its own
app store uses the global web store. A requested relative path is decoded once,
resolved with the store's canonical filesystem path, and compared by path
components. Only a regular file inside that store is sent to the client.

Absolute paths, paths escaping the store through `..`, embedded NUL characters,
and symlinks resolving outside the store return HTTP 404. Windows drive-relative
paths and alternate-data-stream names are also rejected. Invalid resource-path
percent escapes return 400. Ordinary nested files and symlinks whose targets remain inside the
store continue to work. Component comparison also rejects a sibling directory
whose name merely starts with the store's name.
The project selector is checked before project lookup/loading for every HTTP
method. It must be one safe filesystem component: empty, dot/parent, rooted,
control-containing, slash/backslash and colon names are rejected, including
percent-encoded forms. Win32 device names and trailing-dot/space aliases are
also rejected. Existing dotted and Unicode project names remain usable; this
is independent of the policy for creating new projects. Invalid project
selectors return 404 before any project is loaded or any directories are created.
Project resource paths come from parsed URI components, so namespaces containing
the project name and resource query parameters do not become filename text.

The `_cover_image` selection uses the same check and skips outside-store files
rather than exposing a symlink target. The configured store may itself resolve
through a symlink: its canonical target is the store's boundary.

This check controls the resource selected by an HTTP path. Store files and
configuration remain trusted local inputs; it does not provide atomic protection
against a privileged local process replacing filesystem entries between path
resolution and opening the file. The project-selector check applies to every
HTTP method; per-resource checks here cover static reads. Upload/deletion
handler confinement and application authorization are separate changes.

`test_static_resource_containment` runs through the native project manager with
fresh temporary stores and a private sentinel outside them. It covers normal
resources, fallback project app resources, encoded traversal, absolute and NUL
paths, file and directory symlink escapes, allowed in-store symlinks, and cover
selection. It also checks parent-directory project selection is refused before
any parent data/image directories are created, and dotted/Unicode project reads
remain available. It needs permission to create native symlinks and runs without an
existing index or image collection. All checks remain active in Release builds.
