# Configuration

[Back to the README](../README.md)

Edit `SH2DiscordPresence.json` beside the plugin. Settings reload at the polling interval, so you don't need to restart the game. Invalid settings disconnect the mod's presence and write an error to `SH2DiscordPresence.log`.

| Setting                 | Default               | What it does                                                          |
| ----------------------- | --------------------- | --------------------------------------------------------------------- |
| `enabled`               | `true`                | Turns the presence on or off.                                         |
| `application_id`        | `1560708389796061245` | Public Discord Application ID. You can replace it with your own.      |
| `poll_interval_seconds` | `2`                   | Local polling interval, from 1 to 120 seconds.                        |
| `discord_pipe`          | `-1`                  | Finds Discord automatically. Set 0–9 to select a particular instance. |
| `default_image`         | Steam cover URL       | Public HTTPS image URL or a Discord asset key.                        |
| `image_rules`           | `[]`                  | Rules for choosing an image from the activity text.                   |

## Language

Language selection is automatic: **Portuguese game → Portuguese RPC; any other game language → English RPC.** This includes English, French, German, Spanish, Japanese and the other game languages. No translator account, API key or internet translation request is needed.

The mod reads the saved `Language` setting in the current player's `PlayerProfile.sav`, without modifying the file. In this game, the default English setting is omitted from a valid profile; that is handled as English. If the profile can't be read, Steam's current game language is used instead. The Steam client's own UI language doesn't override a readable in-game setting.

The translation is built for this game's activity, not a general-purpose machine translator. Recognized menu messages are translated, and the stable `gamezone` identifier supplies the English location even when Steam's visible text is in another language. Known region names get their normal spelling; other readable English identifiers are split into words. Otherworld is added only when explicitly identified, not inferred from story progress. Portuguese activity text is kept when available.

If no usable activity or location is available, the mod shows `Jogando SILENT HILL 2` for Portuguese or `Playing SILENT HILL 2` for other languages. An undetected language defaults to English. It doesn't keep an old location or send unknown localization tokens as place names. A newly applied language can take time to reach the saved profile and Steam activity.

See the [region catalog](REGIONS.md) for the 15 supported names, their Portuguese labels and the distinction between name coverage and verified in-game detection. Nightfall, Otherworld and Labyrinth subareas have separate entries.

## Custom artwork

You can use public HTTPS image URLs, or upload assets to your own Discord application and use their keys. When using uploaded assets, `application_id` must match the application that owns them.

Add rules to the existing `image_rules` array. For example:

```json
"image_rules": [
  {"contains": ["wood side", "outro mundo"], "image": "wood_side_otherworld"},
  {"contains": ["wood side"], "image": "wood_side"}
]
```

These are sample expressions and asset names, not bundled artwork or confirmed wording for every area. Replace them with words from your actual Steam activity and images you've supplied.

Every expression in a rule must appear together in either the original Steam activity or the translated RPC text. The first matching rule wins, so put specific rules before general ones. Matching ignores ASCII letter case. You can use English rules for translated output, or retain rules that match the original Steam wording.

If no rule matches, the mod uses `default_image`.

## Logs

The log is stored beside the `.asi` and rotates at about 1 MiB. Useful messages:

- `Using the game's local Steam presence.` — local activity is available.
- `Discord confirmed activity` — Discord accepted the text shown on that line.
- `Local Steam presence not ready` — the game hasn't initialized Steam yet, or its Steam interfaces aren't supported. The rest of the line gives the reason.
- `Detected game language` — the in-game setting, or Steam's fallback when the profile isn't readable. No Steam ID or profile path is included.

Making your Steam profile public won't fix a local API issue and isn't required. If the game clears its status, the mod uses the generic text rather than keeping an old location.
