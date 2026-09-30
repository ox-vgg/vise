# Running VISE without a desktop window on Windows

Start `VISE.exe --headless` to run the normal HTTP server without creating the
desktop launcher window. The flag is an exact command-line argument. Interactive
launches without it retain the existing launcher behavior. VISE also chooses this
mode automatically when Windows reports that the process is running in Session 0.

Both modes use the existing VISE settings in `%LOCALAPPDATA%\VISE\vise_settings.txt`
and the same HTTP address, port, and project directory. Initialize or provision the
settings for the account used by the supervisor before starting the process.

The headless process blocks while serving HTTP. A server startup exception exits
with status 1 instead of opening a dialog. It can be supervised by a process/service
wrapper; this flag does not register a Windows service or implement the Service
Control Manager protocol. There is no launcher window to close, so the supervisor
owns the process lifetime.

The Windows integration test `scripts/test/test_windows_headless.py VISE.exe`
uses a temporary `LOCALAPPDATA`, an empty project store, and a separate loopback
port. It checks HTTP serving and verifies that the child process has no top-level
desktop windows. The explicit flag is tested from an interactive session; running
under an actual Session 0 supervisor is a separate deployment check.
