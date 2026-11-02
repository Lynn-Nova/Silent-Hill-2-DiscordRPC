# Silent Hill 2 Discord Presence

A mod that shows your **Silent Hill 2 (2024) Steam activity on Discord**. It displays your current location, region artwork and how long you've been playing this session.

Install it once and launch the game as usual. There's no extra app to open or window to leave running.

[Leia em português](README.pt-BR.md)

## What it shows

- The location or activity reported by the game.
- Regional scenery images, with the game's cover as a fallback and support for custom images.
- A session timer that keeps counting when you change areas.

The mod reads the game's local Steam presence every two seconds and uses its activity and region identifiers to follow your location. Otherworld differences depend on what the game reports; the mod doesn't guess from your progress.

## Installation

You'll need **Silent Hill 2 (2024) on Steam** and the **Discord desktop app**.

1. Close the game and download `SH2DiscordPresence-0.1.6-win64.zip` from this repository's **Releases** section. Choose the mod ZIP, not GitHub's automatic "Source code" download.
2. In Steam, open **Silent Hill 2 → Properties → Installed Files → Browse**.
3. Go to `SHProto/Binaries/Win64`.
4. Extract the ZIP there, next to `SHProto-Win64-Shipping.exe`. Keep the included license files too.
5. Open Discord and start the game through Steam.

**Already using other mods?** The download includes Ultimate ASI Loader. If you already have a compatible loader, reuse it rather than overwriting another mod's DLL or configuration. The presence plugin needs its `.asi` and `.json` files in a folder that loader reads. See the [loader notes](docs/DEVELOPMENT.md#loader-notes) if you're combining setups.

## A few things to know

- **Location changes aren't always instant.** After loading a save or entering an area, the game may take a little while to update its activity.
- **The RPC language is automatic.** Play in Portuguese and your presence stays in Portuguese. For English or any other game language, the mod translates the activity into English. This works offline using the game's activity and region identifiers, with no translation service or API key.
- **The timer is for this session.** It includes menus and pauses, not your save's playtime or total Steam hours.
- **You don't need a public Steam profile.** The mod reads locally. No Steam API key, password or Discord token is needed.
- **Regional artwork is included.** Thirteen scenery-only images cover the listed regions and the approach road: three screenshots and ten official concept artworks, without visible characters or enemies. Normal areas, Nightfall and Otherworld variants have separate images. Nightfall is represented by a Historical Society interior. These are not the original save-menu thumbnails. Menus and unknown locations keep the cover. See the [artwork collection and sources](assets/regions/README.md) for coverage and hosting details.

The mod doesn't edit saves or game executables, and it stops with the game. It doesn't need UE4SS or a separate background program.

## Regions

The [region catalog](docs/REGIONS.md) lists 12 regions, with separate entries for Nightfall and Otherworld locations. The catalog has automated tests, but not every area has been visited in-game.

## Settings

The included settings are ready to use. If you want to change them, edit `SH2DiscordPresence.json` beside the `.asi` file. Changes are picked up automatically.

See [configuration](docs/CONFIGURATION.md) for custom artwork, polling settings and using your own Discord application.

## Something not working?

**Only seeing the generic "Playing" or "Jogando" text?** Load your save and wait for the activity to update. If it stays the same, try moving to another area.

**Nothing showing on Discord?** Check that the desktop app is open and activity sharing is enabled.

**Game in English, presence in Portuguese?** Steam's activity text can follow a different language from the game. The mod reads your saved in-game language first, so a Portuguese Steam client won't force a Portuguese RPC. After applying a language change, give the game time to save the setting and refresh its activity.

If it's still not working, check `SH2DiscordPresence.log` beside the plugin. `Discord confirmed activity` means Discord accepted the text on that line. When reporting a problem, include the mod version, your Steam game build and the relevant log lines. Check anything you share for personal information first.

## Uninstalling

Close the game, then remove `SH2DiscordPresence.asi`, `SH2DiscordPresence.json` and its log files. Only remove `winmm.dll` and `winmm.ini` if they came from this download and no other mod uses them.

## Building from source

With the Windows C++ build tools, CMake and Python installed:

```powershell
./scripts/build.ps1
./scripts/package.ps1
```

The [development notes](docs/DEVELOPMENT.md) cover the full requirements, tests and how the plugin works.

## About this release

**v0.1.6 is a beta update.** It adds the missing artwork for Western South Vale, Nightfall, Otherworld Hospital, Eastern South Vale: Otherworld and Lakeview Hotel: Otherworld. Each variant keeps its own image in Portuguese and English. The previous location-name fixes are included. Update the JSON as well as the plugin to use the new images. Automated tests cover the selection rules; the new images have not been checked in a live game session. Earlier menu, Blue Creek and western city checks used Steam build **17080069**. Other regions, transitions and combinations with mods still need more testing.

## Credits

Created by **Lynn**. Released under the [MIT License](LICENSE).

The project uses [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) and [nlohmann/json](https://github.com/nlohmann/json). Their licenses and other notices are listed in [third-party notices](THIRD_PARTY_NOTICES.md).

An unofficial fan project, not affiliated with Konami, Bloober Team, Valve or Discord. Game names and artwork belong to their respective owners. The code is MIT-licensed; the included game images are not. Regional images are loaded from the project's public GitHub files, and the fallback cover is loaded from Steam.
