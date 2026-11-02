# Region catalog

[Back to the README](../README.md)

The main region list contains these 12 locations, with English and Portuguese labels. Nightfall stays separate from Otherworld.

| English RPC label              | Portuguese label when a translated label is needed |
| ------------------------------ | -------------------------------------------------- |
| Eastern South Vale             | Leste de South Vale                                |
| Wood Side Apartments           | Edifício Wood Side                                 |
| Blue Creek Apartments          | Edifício Blue Creek                                |
| Western South Vale             | Oeste de South Vale                                |
| Brookhaven Hospital            | Hospital Brookhaven                                |
| Otherworld Hospital            | Hospital Brookhaven: Outro Mundo                   |
| Eastern South Vale: Otherworld | Leste de South Vale: Outro Mundo                   |
| Western South Vale: Nightfall  | Oeste de South Vale: Anoitecer                     |
| Toluca Prison                  | Prisão Toluca                                      |
| Labyrinth                      | Labirinto                                          |
| Lakeview Hotel                 | Hotel Lakeview                                     |
| Lakeview Hotel: Otherworld     | Hotel Lakeview: Outro Mundo                        |

The reference spells Blue Creek as "Bluecreek"; both forms map to the same entry. Portuguese labels in this table are the mod's wording, not a claim about official localization. The original Portuguese Steam text is kept when available, with the hotel naming exception described below.

## What has actually been checked?

**Catalog coverage is not the same as a full playthrough.** Automated tests check every row, several input aliases, English output for English/French/Japanese game settings, Portuguese output, and an unchanged session timer. Separate checks keep Nightfall and Otherworld labels and images distinct.

During development, the menu, Blue Creek and the western city area were checked in-game. The western city log showed `está na zona:Zona Oeste da Cidade`. `BluecreekApartments` is the raw region identifier actually captured from Steam. The other names and aliases are supported inputs based on the region list, not independently captured Steam identifiers. The catalog has not been validated by visiting every area.

## How a region is matched

Spaces, underscores, hyphens, colons and parentheses are ignored when matching known names. Matching ignores ASCII letter case. For example, `Eastern South Vale: Otherworld` and `EasternSouthValeOtherworld` select the same label. Some explicit aliases, such as `BrookhavenHospital_OW`, are accepted too.

`WestCity` / `West City` map to **Western South Vale**, and `EastCity` / `East City` map to **Eastern South Vale**. These names are also standardized when they occur in English activity text without a separate `gamezone` value. Explicit Otherworld and Nightfall aliases remain separate; the mod never treats those two words as interchangeable.

`NightCityOW`, `NightCity_OW` and `Night City OW` map to **Eastern South Vale: Otherworld**, matching the in-game location reported with that activity text. This is not Western South Vale: Nightfall. The mapping has automated regression coverage; it does not mean every Otherworld transition has been checked in-game.

`HotelFW`, `Hotel_FW`, `Hotel FW` and `Hotel` map to **Lakeview Hotel**. Explicit Otherworld names such as `HotelOW`, `Hotel_OW`, `Hotel OW` and `Hotel do Outro Mundo` map to **Lakeview Hotel: Otherworld**. Portuguese hotel activity uses **Hotel Lakeview** or **Hotel Lakeview: Outro Mundo**, preserving its original activity prefix. Other Portuguese locations keep the game's wording.

Outside the main region list, the approach road is also named explicitly: `RoadtoSilentHill`, `RoadTo Silent Hill` and separator variants map to **Road To Silent Hill** in English or **Estrada para Silent Hill** in Portuguese. The hotel and road mappings have regression tests, including activity text without a separate `gamezone` field. Their reported activity text was observed in logs; these aliases are not a claim that every raw identifier was independently captured from Steam.

The mod still depends on the `gamezone` value the game publishes. If an area uses a different internal identifier, another alias may be needed. It doesn't guess from saves, story progress or coordinates.

Unknown readable English identifiers are formatted as words; unavailable or unusable activity uses the generic localized fallback. The [artwork collection](../assets/regions/README.md) includes thirteen scenery-only images for all twelve listed regions and the approach road, combining screenshots with official concept art. They do not reproduce the save-menu thumbnails; Nightfall is represented by a Historical Society interior. Unknown areas and menus retain the cover. Image rules can match the canonical English region name independently of the displayed language. See [image rules](CONFIGURATION.md#custom-artwork) to customize them.
