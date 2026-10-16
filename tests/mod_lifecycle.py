import json
import pathlib
import shutil
import subprocess
import sys
import tempfile


with tempfile.TemporaryDirectory(prefix="sh2presence-test-") as temporary:
    folder = pathlib.Path(temporary)
    host = folder / "SHProto-Win64-Shipping.exe"
    plugin = folder / "SH2DiscordPresence.asi"
    shutil.copy2(sys.argv[1], host)
    shutil.copy2(sys.argv[2], plugin)
    plugin.with_suffix(".json").write_text(json.dumps({"enabled": False}), encoding="utf-8")
    result = subprocess.run([str(host), str(plugin)], capture_output=True, text=True, timeout=25)
    print(result.stdout)
    if result.returncode:
        raise RuntimeError(result.stderr)
    log = plugin.with_suffix(".log").read_text(encoding="utf-8")
    assert log.count("Native ASI plugin") == 1, log
    assert "Presence stopped and IPC connection closed." in log, log
