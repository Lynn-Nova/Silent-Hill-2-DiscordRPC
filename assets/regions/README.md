# Region artwork

[Back to the README](../../README.md)

Thirteen unmodified environment images are included: three promotional screenshots and ten official concept artworks. They were visually reviewed to select scenery without visible characters, enemies or gameplay HUD. Source URLs, image types and SHA-256 checksums are recorded in [sources.json](sources.json). Game images belong to Konami and their respective creators and are **not** covered by the mod's MIT license. This source record is not a separate permission or an endorsement by the publisher.

## Images

| Region | File | Source |
| --- | --- | --- |
| Eastern South Vale | [Preview](eastern-south-vale.jpg) | [Konami screenshot 5](https://www.konami.com/games/silenthill/2r/s/img/guideline/available_sh2_ss_5.jpg) |
| Western South Vale | [Preview](western-south-vale.webp) | [Official environment art: Silent Hill Street 01, Jakub Skop](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Western South Vale: Nightfall | [Preview](western-south-vale-nightfall.webp) | [Official environment art: Historical Society 03, Andreas Stavaas](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Otherworld Hospital | [Preview](brookhaven-hospital-otherworld.webp) | [Official environment art: Otherworld Hospital 07, Konrad Białowąs](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Eastern South Vale: Otherworld | [Preview](eastern-south-vale-otherworld.jpg) | [Official concept art: Otherworld South Vale 03](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Lakeview Hotel: Otherworld | [Preview](lakeview-hotel-otherworld.webp) | [Official environment art: Otherworld Hotel 01, Konrad Białowąs](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Road To Silent Hill | [Preview](road-to-silent-hill.jpg) | [Official concept art: Path to Silent Hill 02](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Blue Creek Apartments | [Preview](blue-creek-apartments.jpg) | [Official concept art: Blue Creek 12](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Brookhaven Hospital | [Preview](brookhaven-hospital.jpg) | [Official concept art: Brookhaven 03](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Wood Side Apartments | [Preview](wood-side-apartments.jpg) | [Official concept art: Wood Side 15](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Toluca Prison | [Preview](toluca-prison.jpg) | [Promotional gallery, 2024-09-23](https://www.silenthillmemories.net/sh2_remake/screenshots_en.htm) |
| Labyrinth | [Preview](labyrinth.webp) | [Official concept art: Labyrinth Center 02, Aneta Bielas](https://www.silenthillmemories.net/sh2_remake/art_en.htm) |
| Lakeview Hotel / Hotel FW | [Preview](lakeview-hotel.jpg) | [Promotional gallery, 2024-10-03](https://www.silenthillmemories.net/sh2_remake/screenshots_en.htm) |

These are representative environment images, **not the original save-menu thumbnails** or live captures of the player's exact room. Concept art may differ from the final game. Original artist credits and borders are preserved where present; no characters were removed by AI and no game images were generated. The Labyrinth uses one shared image. The no-character selection applies to these thirteen regional assets, not the existing Steam cover used for menus and unknown areas.

Western South Vale uses a foggy residential street. Nightfall uses a dark underground room from the Historical Society, representing the nighttime return to the area; it is not an outdoor night screenshot. Each listed region now has an image, including separate Otherworld variants. Nightfall and Otherworld rules come before the base-region rules so the normal-area image cannot override them. Unknown locations and menus use the cover.

## Publishing and display

The shipped JSON refers to files in the public `main` branch of `Lynn-Nova/Silent-Hill-2-DiscordRPC`. **Publish this whole `assets/regions` folder with the updated source before distributing a release.** If those files are missing or the repository becomes private, the regional image URLs will fail and Discord cannot display them.

Discord fetches the HTTPS images; it does not load these files from the player's game directory. The archive copies preserve the source material and credits, but are not a local-image fallback. If a remote URL stops working, use another public image URL or upload the files as assets to the Discord application and put their asset keys in the JSON. The mod cannot detect an image-fetch failure inside Discord.

Before publishing or replacing game imagery, review [Konami's screenshot guidelines](https://www.konami.com/games/silenthill/2r/guideline/jp/ja/) and any applicable source and artist conditions. The gameplay screenshot guidelines are not a blanket license for concept art. Do not treat these images as MIT-licensed assets or replace them with another creator's work without permission.
