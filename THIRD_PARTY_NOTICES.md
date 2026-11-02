# Third-party components

## nlohmann/json 3.12.0

Source: https://github.com/nlohmann/json/tree/v3.12.0

The vendored single header is `third_party/nlohmann/json.hpp` (SHA-256 `aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63`). Distributed under MIT; the original license is in `third_party/nlohmann/LICENSE.MIT`, or `licenses/nlohmann-json.LICENSE` in binary packages.

## Ultimate ASI Loader 9.7.4

Source: https://github.com/ThirteenAG/Ultimate-ASI-Loader/tree/v9.7.4

Release archive: `Ultimate-ASI-Loader-NoPDB_x64.zip`.

SHA-256: `e5860e7d9a1805267535b65749575b5e406cc6ea3325c7392189c578815045d1`.

The packaging script downloads this pinned release, verifies its checksum, and renames its `dinput8.dll` to the supported proxy name `winmm.dll`. The loader is MIT licensed; its original license is preserved as `third_party/Ultimate-ASI-Loader.LICENSE` and included in binary packages.

## Zig-built binaries

The locally built binary uses Zig 0.15.2, its bundled LLVM C++ libraries and MinGW-w64 runtime support. Their original notices are in `third_party/toolchain`, copied to `licenses` in binary packages. These compiler/runtime notices do not change the MIT license of the mod's own code. A build made with MSVC uses the corresponding Microsoft runtime instead.

## Artwork

Thirteen unmodified environment images are included in `assets/regions`: three promotional screenshots and ten official concept artworks. Sources, image types and checksums are recorded in `assets/regions/sources.json`. The Eastern South Vale screenshot comes directly from Konami's downloadable collection; the other images are archived by Silent Hill Memories. Original credits are retained, including those of Aneta Bielas, Jakub Skop, Andreas Stavaas and Konrad Białowąs. See the [artwork notes](assets/regions/README.md) for individual sources, coverage, hosting and publisher guidelines. These images are not licensed under this project's MIT license, and source attribution does not itself grant reuse permission or imply endorsement. Gameplay screenshot guidelines are not a blanket license for concept artwork. Game names, artwork and trademarks retain their respective ownership. The fallback cover is loaded from Steam and is not included in the archives.
