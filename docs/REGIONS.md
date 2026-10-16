# Region catalog

[Back to the README](../README.md)

The mod includes these 15 named regions, with English and Portuguese labels. Nightfall stays separate from Otherworld, and the three Labyrinth subareas stay separate from the main Labyrinth.

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
| Labyrinth: Rotten Area         | Labirinto: Área Apodrecida                         |
| Labyrinth: Desolate Area       | Labirinto: Área Desolada                           |
| Labyrinth: Ruined Area         | Labirinto: Área em Ruínas                          |
| Lakeview Hotel                 | Hotel Lakeview                                     |
| Lakeview Hotel: Otherworld     | Hotel Lakeview: Outro Mundo                        |

The reference spells Blue Creek as "Bluecreek"; both forms map to the same entry. Portuguese labels in this table are the mod's wording, not a claim about official localization. The original Portuguese Steam text is kept when available.

## What has actually been checked?

**Catalog coverage is not the same as a full playthrough.** Automated tests check every row, several input aliases, English output for English/French/Japanese game settings, Portuguese output, and an unchanged session timer. Separate checks keep Nightfall and Otherworld distinct and check subarea-specific artwork rules.

During development, the menu, Blue Creek and the western city area were checked in-game. The western city log showed `está na zona:Zona Oeste da Cidade`. `BluecreekApartments` is the raw region identifier actually captured from Steam. The other names and aliases are supported inputs based on the region list, not independently captured Steam identifiers. The catalog has not been validated by visiting every area.

## How a region is matched

Spaces, underscores, hyphens, colons and parentheses are ignored when matching known names. Matching ignores ASCII letter case. For example, `Eastern South Vale: Otherworld` and `EasternSouthValeOtherworld` select the same label. Some explicit aliases, such as `BrookhavenHospital_OW`, are accepted too.

Explicit Otherworld and Nightfall aliases remain separate; the mod never treats those two words as interchangeable.

The mod still depends on the `gamezone` value the game publishes. If an area uses a different internal identifier, another alias may be needed. If Steam only reports `Labyrinth`, the mod cannot determine which Labyrinth subarea you're in. It doesn't guess from saves, story progress or coordinates.

Unknown readable English identifiers are formatted as words; unavailable or unusable activity uses the generic localized fallback. A region catalog doesn't add per-area images: artwork is still the cover by default, unless you configure your own [image rules](CONFIGURATION.md#custom-artwork).
