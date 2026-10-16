import pathlib
import shutil
import subprocess
import sys
import tempfile


with tempfile.TemporaryDirectory(prefix="sh2presence-steam-test-") as temporary:
    folder = pathlib.Path(temporary)
    host = folder / "steam_reader_test.exe"
    stub = folder / "steam_api64.dll"
    shutil.copy2(sys.argv[1], host)
    shutil.copy2(sys.argv[2], stub)
    result = subprocess.run([str(host), str(stub)], capture_output=True, text=True, timeout=15)
    print(result.stdout)
    if result.returncode:
        raise RuntimeError(result.stderr)
