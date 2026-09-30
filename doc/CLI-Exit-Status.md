# Index creation exit status

`vise-cli --cmd=create-project NAME:/path/to/project/data/conf.txt` blocks until
index creation finishes. It exits with status 0 when `index_create()` reports
success and status 1 when that operation reports failure. The same rule applies
to `--cmd=create-visual-vocabulary`. Both commands require exactly one project
argument and an existing configuration file named `conf.txt`.

Supervisors and scripts should check the exit status and verify the completed
index files before publishing the project. A reported failure (including an
already-created index rejected by `index_create()`) is an error status; these
commands do not promise idempotent success for an existing index. Some failure
paths can leave partial files for inspection. This change does not remove them
or change index creation itself.

`scripts/test/test_cli_index_status.py /path/to/vise-cli` checks the error contract
using only a temporary directory and an invalid configuration filename. It covers
both creation commands, missing files and arguments, and the successful version
command without building a real image index.
