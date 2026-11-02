import hashlib
import json
import pathlib
import struct
import urllib.parse


project = pathlib.Path(__file__).resolve().parents[1]
directory = project / "assets" / "regions"
manifest = json.loads((directory / "sources.json").read_text(encoding="utf-8"))
settings = json.loads((project / "config" / "SH2DiscordPresence.json").read_text(encoding="utf-8"))
image_base = manifest["image_base"]
assert image_base == "https://raw.githubusercontent.com/Lynn-Nova/Silent-Hill-2-DiscordRPC/main/assets/regions/"
assert len(manifest["images"]) == 8
assert sum(entry["kind"] == "screenshot" for entry in manifest["images"]) == 3
assert sum(entry["kind"] == "concept_art" for entry in manifest["images"]) == 5
files = set()
for entry in manifest["images"]:
    filename = entry["file"]
    assert pathlib.PurePosixPath(filename).name == filename
    assert filename not in files
    files.add(filename)
    data = (directory / filename).read_bytes()
    assert 1024 < len(data) < 8 * 1024 * 1024
    assert hashlib.sha256(data).hexdigest() == entry["sha256"], filename
    if filename.endswith(".jpg"):
        assert data.startswith(b"\xff\xd8\xff") and data.endswith(b"\xff\xd9"), filename
    elif filename.endswith(".webp"):
        assert data[:4] == b"RIFF" and data[8:12] == b"WEBP", filename
        assert struct.unpack("<I", data[4:8])[0] + 8 == len(data), filename
    else:
        raise AssertionError(f"Unexpected image format: {filename}")
    for key in ("source_page", "source_url"):
        source = urllib.parse.urlsplit(entry[key])
        assert source.scheme == "https"
        assert source.hostname in {"www.konami.com", "www.silenthillmemories.net"}
referenced = set()
for rule in settings["image_rules"]:
    url = rule["image"]
    if url == settings["default_image"]:
        continue
    assert url.startswith(image_base), url
    filename = url[len(image_base):]
    assert filename in files, filename
    referenced.add(filename)
assert referenced == files
assert files == {image.name for image in directory.iterdir() if image.suffix in {".jpg", ".webp"}}
print(f"Artwork files: {len(files)} original images (3 screenshots, 5 concept artworks), checksums, sources and configured paths verified.")
