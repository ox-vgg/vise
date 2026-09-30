"""Check CLI indexing failures in a disposable project, without image data."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="vise-cli-status-") as temporary:
        root = Path(temporary)
        data = root / "project" / "data"
        data.mkdir(parents=True)
        # An existing file with the wrong name fails the project's documented
        # conf.txt precondition, then index_create reports success=false.
        invalid = data / "not-conf.txt"
        invalid.write_text("search_engine=relja_retrival\n", encoding="utf-8")
        env = dict(os.environ, LOCALAPPDATA=temporary)
        flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0

        def run(arguments):
            return subprocess.run([executable, *arguments], env=env, capture_output=True,
                                  text=True, timeout=30, creationflags=flags)

        version = run(["--version"])
        if version.returncode != 0:
            raise AssertionError(f"Version command failed: {version.stdout} {version.stderr}")
        for command in ("create-project", "create-visual-vocabulary"):
            failed = run([f"--cmd={command}", f"test:{invalid}"])
            if failed.returncode != 1 or "PRECONDITION FAILED" not in failed.stdout:
                raise AssertionError(f"{command} falsely reported success: {failed.returncode}\n"
                                     f"{failed.stdout}\n{failed.stderr}")
            missing = run([f"--cmd={command}", f"test:{data / 'missing.txt'}"])
            if missing.returncode != 1 or "was not found" not in missing.stdout:
                raise AssertionError(f"{command} did not reject missing configuration")
            no_project = run([f"--cmd={command}"])
            if no_project.returncode != 1 or "only a single" not in no_project.stdout:
                raise AssertionError(f"{command} did not validate its project argument count")
        print("PASS: CLI indexing failure, missing configuration, project count, and version status")


if __name__ == "__main__":
    main()
