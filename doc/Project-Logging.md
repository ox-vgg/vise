# Project log location

The `relja_retrival` search engine appends progress and runtime diagnostics to
`data/index.log` by default. A project configuration (`data/conf.txt`) can set
`index_log_file` to use another file:

```text
index_log_file=/var/log/vise/collection.log
```

On Windows, use an absolute path such as `C:/VISELogs/collection.log`. A relative
path is resolved against the project directory, independent of the process working
directory. For example, `index_log_file=logs/index.log` uses the project's `logs`
directory. An omitted or empty option retains the original `data/index.log` location.

Provision the parent directory and write permissions before starting VISE. VISE
does not create the log directory. A configured destination lets deployments keep
the index directory read-only and put diagnostics in a writable runtime directory.
If the destination cannot be opened, VISE reports the failure on standard error
and continues initialization without file logging. Log files use append mode.

This setting controls logging only. Index creation, configuration changes, cached
weights, and other project operations may still require writable project files;
it does not turn every project operation into a read-only operation.
