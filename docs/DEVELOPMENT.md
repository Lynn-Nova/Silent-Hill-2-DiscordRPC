# Development notes

[Back to the README](../README.md)

## Building

To build on Windows, you'll need CMake 3.21+, the Microsoft C++ build tools and Python 3 for the protocol and lifecycle tests. The code uses C++17 and a pinned, vendored copy of nlohmann/json.

```powershell
./scripts/build.ps1
./scripts/package.ps1
```

The build script compiles the plugin and runs CTest. The packaging script downloads Ultimate ASI Loader v9.7.4, checks its SHA-256 and creates `release/SH2DiscordPresence-0.1.5-win64.zip` with the documentation and licenses. Version 0.1.5 is a beta bug-fix update.

GitHub Actions follows the same workflow and uploads the ZIP as a build artifact. It doesn't push commits or publish a GitHub Release.

### Zig

Zig **0.15.2** is also supported. With this path, run the tests separately:

```powershell
./scripts/build.ps1 -ZigPath 'C:/tools/zig/zig.exe'
./build/core_tests.exe ./config/SH2DiscordPresence.json
python ./tests/artwork_files.py
python ./tests/ipc_tests.py ./build/ipc_test_client.exe
python ./tests/local_steam_tests.py ./build/steam_reader_test.exe ./build/steam_api_stub.dll
python ./tests/mod_lifecycle.py ./build/SHProto-Win64-Shipping.exe ./build/SH2DiscordPresence.asi
./scripts/package.ps1
python ./tests/loader_lifecycle.py ./build/SHProto-Win64-Shipping.exe ./build/SH2DiscordPresence.asi ./build/loader-v9.7.4/dinput8.dll
```

## How presence is read

The plugin uses the game's already loaded `steam_api64.dll` and the read-only [ISteamFriends Rich Presence API](https://partner.steamgames.com/doc/api/ISteamFriends#GetFriendRichPresence). The tested game exposes `SteamAPI_SteamFriends_v017` and `SteamAPI_SteamUser_v021`.

For this game, `steam_display` contains the activity text, and `gamezone` supplies an English region identifier such as `BluecreekApartments`. The plugin prefers `steam_display`, with `status` as a plain-text fallback. Unresolved localization tokens aren't displayed as locations. There are no HTTP requests for status or translation, no Unreal Engine offsets and no calls to initialize or shut down Steam. The plugin doesn't replace Steam DLLs, install hooks or run Steam callbacks.

### Automatic RPC language

The mod reads the `Language` property from `%LOCALAPPDATA%/SilentHill2/Saved/SaveGames/<current-user>/PlayerProfile.sav` using the current Steam user from the loaded SDK. This is a bounded, read-only lookup of that profile, not a scan of other accounts or a save editor. It checks the profile class, terminator and string property before accepting a locale. The game's default English setting is omitted from the serialized profile, so a valid profile without that property means English. Unreadable profiles fall back to `SteamAPI_SteamApps_v008` / `SteamAPI_ISteamApps_GetCurrentGameLanguage` when available.

Portuguese locales use Portuguese output; other locales use offline English activity formatting. Steam's visible status can remain in Portuguese even with an English game, so output language cannot be selected solely from that text. Region identifiers avoid having to machine-translate arbitrary Steam text. A small built-in name table handles common names; readable English identifiers can also be split into words. Unknown states use a localized generic fallback. This is not a universal free-text translator.

## Loader notes

The release includes Ultimate ASI Loader **v9.7.4**, with its `dinput8.dll` renamed to the supported proxy name `winmm.dll`. The tested layout places the plugin and JSON beside `SHProto-Win64-Shipping.exe`, not in a separate `scripts` folder.

The bundled `winmm.ini` uses:

```ini
[GlobalSets]
LoadPlugins=1
LoadFromScriptsOnly=0
DontLoadFromDllMain=0
LoadRecursively=0
```

These settings also affect other ASI files handled by the same loader. Don't replace another mod's loader or configuration without checking its requirements. If reusing a loader, it must call the plugin's exported `InitializeASI` and load the folder containing the plugin and its JSON.

`DllMain` only stores the module handle. `InitializeASI` creates a stop event and starts a worker. Configuration parsing, module pinning, local Steam reads and Discord I/O happen on that worker after Windows finishes DLL thread notifications.

The module stays pinned for the host process's lifetime. `ShutdownASI` is available to the test harness, but hot unloading in a running game isn't supported. Initialization is restricted to the remake's `SHProto-Win64-Shipping.exe` process name. Discord named-pipe I/O uses overlapped operations with cancellation and deadlines.

## Tests

The core test receives the shipped configuration and checks default artwork across the localized region aliases. Image rules match original text, displayed text or the canonical English region derived from the active Steam `gamezone`; recognized menu activity suppresses stale-region matching. `tests/artwork_files.py` checks all eight original image files, source records, hashes and configured GitHub paths without network access. These checks do not prove that future GitHub URLs have already been published or that Discord displayed every image. Publish `assets/regions` with the source before creating the release; see the [artwork notes](../assets/regions/README.md).

The [region catalog](REGIONS.md) includes 15 region names. `tests/region_catalog.hpp` supplies cases for every name, common spelling and separator variants, and explicit aliases. Tests check English output for English/French/Japanese game settings, Portuguese output, timer continuity, distinct Nightfall/Otherworld and Labyrinth areas, and specific artwork precedence. These fixtures do not establish what identifiers Steam actually sends in unvisited areas; only `BluecreekApartments` was captured from the live game.

The mod displays `West City` as **Western South Vale** and `East City` as **Eastern South Vale**, including English activity text without a `gamezone` field. These name mappings are covered by automated tests, not a full in-game playthrough.

The reported `Night City OW` activity maps to **Eastern South Vale: Otherworld**. Regression tests cover spaced, compact and underscore forms, English activity text without a region field, Portuguese Steam text with English output, and preservation of Portuguese output. This alias does not change the separate Western South Vale: Nightfall entry.

Hotel tests cover `HotelFW` and `HotelOW` aliases as distinct Lakeview locations, including the Portuguese activity text observed in logs. Portuguese hotel names are normalized while keeping their activity prefix and spacing. Other Portuguese locations retain the original text. Artwork rules still match either the original activity or its normalized output. The approach road also has explicit spacing tests for `RoadtoSilentHill` and its variants. These are automated checks, not a new in-game playthrough of the updated plugin.

The automated checks cover local Steam reads with a test double, saved-language parsing (including the omitted English default), profile precedence over Steam, locale changes, Portuguese/English activity formatting, UTF-8 handling, artwork rules, Discord framing, partial reads, timeouts, disconnections and plugin lifecycle. Profile tests use synthetic files inside a temporary test directory. `tests/loader_lifecycle.py` also checks the actual bundled loader and its configuration.

Lifecycle tests use a separately built host named `SHProto-Win64-Shipping.exe` in a temporary directory. They don't launch or change the real game. The Steam API test double is also loaded only in a temporary folder: **never install it in a game directory**. It isn't part of the release ZIP.

During development, in-game checks used Steam build **17080069**. Discord accepted the menu text and `está na zona:Edifício Blue Creek` after loading a save. With the game in English and Steam still in Portuguese, the mod detected the saved language and Discord accepted `Preparing to explore Silent Hill`, followed by `Exploring Blue Creek Apartments`. Switching back to Portuguese also updated the presence correctly. The western city area was confirmed with `está na zona:Zona Oeste da Cidade`. These checks do not amount to an in-game validation of every region in the release catalog. Other regions, Otherworld transitions, game builds and mod combinations need further playtesting.
