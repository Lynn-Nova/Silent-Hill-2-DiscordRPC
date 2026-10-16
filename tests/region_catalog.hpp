#pragma once

#include <initializer_list>

struct RegionFixture {
    const char* english;
    const char* portuguese;
    std::initializer_list<const char*> aliases;
};

inline const RegionFixture region_fixtures[] = {
    {"Eastern South Vale", "Leste de South Vale", {"Eastern South Vale", "EasternSouthVale", "EastSouthVale", "EastCity", "East City"}},
    {"Wood Side Apartments", u8"Edifício Wood Side", {"Wood Side Apartments", "WoodsideApartments", "wood_side_apartments"}},
    {"Blue Creek Apartments", u8"Edifício Blue Creek", {"Bluecreek Apartments", "BluecreekApartments", "Blue Creek Apartments"}},
    {"Western South Vale", "Oeste de South Vale", {"Western South Vale", "WesternSouthVale", "WestSouthVale", "WestCity", "West City"}},
    {"Brookhaven Hospital", "Hospital Brookhaven", {"Brookhaven Hospital", "BrookhavenHospital", "brookhaven_hospital"}},
    {"Otherworld Hospital", "Hospital Brookhaven: Outro Mundo", {"Otherworld Hospital", "OtherworldHospital", "BrookhavenHospitalOtherworld", "BrookhavenHospital_OW", "HospitalOW"}},
    {"Eastern South Vale: Otherworld", "Leste de South Vale: Outro Mundo", {"Eastern South Vale: Otherworld", "EasternSouthValeOtherworld", "EastSouthValeOtherworld", "EasternSouthVale_OW", "EastCityOtherworld", "EastCity_OW"}},
    {"Western South Vale: Nightfall", "Oeste de South Vale: Anoitecer", {"Western South Vale: Nightfall", "WesternSouthValeNightfall", "WestSouthValeNightfall", "WestCityNightfall"}},
    {"Toluca Prison", u8"Prisão Toluca", {"Toluca Prison", "TolucaPrison", "toluca_prison"}},
    {"Labyrinth", "Labirinto", {"Labyrinth", "labyrinth", "LABYRINTH"}},
    {"Labyrinth: Rotten Area", u8"Labirinto: Área Apodrecida", {"Labyrinth: Rotten Area", "LabyrinthRottenArea", "LabyrinthRotten", "RottenArea"}},
    {"Labyrinth: Desolate Area", u8"Labirinto: Área Desolada", {"Labyrinth: Desolate Area", "LabyrinthDesolateArea", "LabyrinthDesolate", "DesolateArea"}},
    {"Labyrinth: Ruined Area", u8"Labirinto: Área em Ruínas", {"Labyrinth: Ruined Area", "LabyrinthRuinedArea", "LabyrinthRuined", "RuinedArea"}},
    {"Lakeview Hotel", "Hotel Lakeview", {"Lakeview Hotel", "LakeviewHotel", "lakeview_hotel"}},
    {"Lakeview Hotel: Otherworld", "Hotel Lakeview: Outro Mundo", {"Lakeview Hotel: Otherworld", "LakeviewHotelOtherworld", "OtherworldLakeviewHotel", "LakeviewHotel_OW"}}
};

static_assert(sizeof(region_fixtures) / sizeof(region_fixtures[0]) == 15);
