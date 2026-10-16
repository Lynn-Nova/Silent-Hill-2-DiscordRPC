import json
import pathlib
import shutil
import subprocess
import sys
import tempfile


with tempfile.TemporaryDirectory(prefix="sh2presence-loader-") as temporary:
    folder = pathlib.Path(temporary)
    host = folder / "SHProto-Win64-Shipping.exe"
    plugin = folder / "SH2DiscordPresence.asi"
    loader = folder / "winmm.dll"
    shutil.copy2(sys.argv[1], host)
    shutil.copy2(sys.argv[2], plugin)
    shutil.copy2(sys.argv[3], loader)
    plugin.with_suffix(".json").write_text(json.dumps({"enabled": False}), encoding="utf-8")
    project = pathlib.Path(__file__).resolve().parents[1]
    shutil.copy2(project / "config" / "winmm.ini", folder / "winmm.ini")
    result = subprocess.run([str(host), "--loader", str(loader)], cwd=folder, capture_output=True, text=True, timeout=25)
    print(result.stdout)
    if result.returncode:
        raise RuntimeError(result.stderr)
    log = plugin.with_suffix(".log").read_text(encoding="utf-8")
    assert log.count("Native ASI plugin") == 1, log
    assert "Presence stopped and IPC connection closed." in log, log
