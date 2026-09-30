"""Run a Windows VISE executable with an isolated home and no desktop window."""
import ctypes
from ctypes import wintypes
import http.client
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time


def windows_for_process(pid):
    found = []
    callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    user32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
    user32.EnumWindows.argtypes = [callback_type, wintypes.LPARAM]

    @callback_type
    def collect(hwnd, unused):
        owner = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if owner.value == pid:
            found.append(hwnd)
        return True

    if not user32.EnumWindows(collect, 0):
        raise ctypes.WinError(ctypes.get_last_error())
    return found


def main():
    if os.name != "nt":
        print("Windows-only integration test")
        return
    executable = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="vise-headless-") as temporary:
        root = Path(temporary) / "VISE"
        for folder in (root, root / "project", root / "www", root / "asset"):
            folder.mkdir(exist_ok=True)
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        settings = {
            "vise-home-dir": root,
            "vise-project-dir": root / "project",
            "vise-asset-dir": root / "asset",
            "http-www-dir": root / "www",
            "http-address": "127.0.0.1",
            "http-port": port,
            "http-worker": 1,
            "http-namespace": "/",
        }
        (root / "vise_settings.txt").write_text(
            "".join(f"{key}={value}\n" for key, value in settings.items()), encoding="utf-8"
        )
        (root / "www" / "probe.txt").write_text("headless-test-ok", encoding="ascii")
        env = dict(os.environ, LOCALAPPDATA=temporary)
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        process = subprocess.Popen([executable, "--headless"], env=env, startupinfo=startup)
        try:
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise AssertionError(f"VISE exited before serving HTTP: {process.returncode}")
                try:
                    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=1)
                    connection.request("GET", "/probe.txt")
                    response = connection.getresponse()
                    body = response.read()
                    connection.close()
                    if response.status == 200 and body == b"headless-test-ok":
                        break
                except OSError:
                    pass
                time.sleep(0.1)
            else:
                raise AssertionError("VISE did not serve the isolated HTTP probe")
            if windows_for_process(process.pid):
                raise AssertionError("--headless created a top-level desktop window")
            print("Headless HTTP serving and absence of desktop windows verified")
        finally:
            # Terminate only the isolated child created by this test.
            if process.poll() is None:
                process.terminate()
            process.wait(timeout=10)
        # A startup failure must exit with an error instead of opening a dialog
        # or waiting forever on the GUI message loop.
        with socket.socket() as occupied:
            occupied.bind(("127.0.0.1", port))
            occupied.listen()
            failed = subprocess.Popen([executable, "--headless"], env=env, startupinfo=startup)
            try:
                if failed.wait(timeout=10) != 1:
                    raise AssertionError("Occupied-port startup must exit with status 1")
                print("Headless startup failure exits with status 1")
            finally:
                if failed.poll() is None:
                    failed.terminate()
                failed.wait(timeout=10)


if __name__ == "__main__":
    main()
