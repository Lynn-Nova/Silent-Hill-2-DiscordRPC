#include "core.hpp"
#include "profile_fixture.hpp"
#include "region_catalog.hpp"
#include <iostream>
#include <stdexcept>

void expect(bool condition, const char* description) {
    if (!condition) throw std::runtime_error(description);
}

int main() {
    try {
        using namespace sh2presence;
        const auto status = parse_miniprofile(R"(<img src='https://cdn.test/apps/2124490/logo.jpg' class='game_logo'><span class='wrapper'><span class='rich_presence extra'> Explorando <b>Wood Side</b><span> Apartments</span> &amp; p&#225;tio &#x1F47B; </span></span>)");
        expect(status.game_id == "2124490", "Game ID extraction");
        expect(status.details == u8"Explorando Wood Side Apartments & pátio 👻", "Nested tags and HTML entities");
        auto config = parse_config(nlohmann::json::parse(R"({"image_rules":[{"contains":["wood side","outro mundo"],"image":"otherworld"},{"contains":["wood side"],"image":"normal"}]})"));
        const auto normal = make_activity(config, {"2124490", "Explorando Wood Side Apartments"}, 1700000000);
        const auto otherworld = make_activity(config, {"2124490", "Explorando Wood Side Apartments (Outro Mundo)"}, 1700000000);
        expect(normal.at("assets").at("large_image") == "normal", "Normal world image");
        expect(otherworld.at("assets").at("large_image") == "otherworld", "Otherworld image precedence");
        expect(normal.at("timestamps") == otherworld.at("timestamps"), "Timer preserved across locations");
        expect(otherworld.at("details") == "Explorando Wood Side Apartments (Outro Mundo)", "Exact Steam status preserved");
        expect(make_activity(config, {"123", "Other game's location"}, 1).at("details") == "Playing SILENT HILL 2", "Other game rejected");
        expect(make_activity(config, {}, 1).at("details") == "Playing SILENT HILL 2", "Unknown language uses English fallback");
        expect(parse_miniprofile("<div>Private profile</div>").details.empty(), "Private profile handled");
        const auto menu = steam_status_from_presence({{"steam_display", u8"está se preparando para se aventurar em Silent Hill"}});
        const auto blue_creek = steam_status_from_presence({{"steam_display", u8"está na zona:Edifício Blue Creek"}, {"gamezone", "BlueCreek"}});
        expect(menu.details == u8"está se preparando para se aventurar em Silent Hill", "Real local menu status");
        expect(blue_creek.details == u8"está na zona:Edifício Blue Creek", "Real local region status from steam_display");
        expect(blue_creek.game_id == "2124490", "Local status belongs to SH2");
        expect(make_activity(config, blue_creek, 1700000000).at("details") == blue_creek.details, "Local region reaches Discord activity");
        const std::string english_text = "Exploring Blue Creek Apartments";
        const auto english_status = steam_status_from_presence({{"steam_display", english_text}});
        expect(english_status.details == english_text, "English presence passes through without translation");
        expect(make_activity(config, english_status, 1700000000).at("details") == english_text, "English presence reaches Discord unchanged");
        const auto bilingual_status = [](const std::string& language) {
            return steam_status_from_presence({{"steam_display", u8"está na zona:Edifício Blue Creek"}, {"gamezone", "BluecreekApartments"}}, language);
        };
        for (const auto& language : {"en", "english", "fr", "de", "ja", "zh-Hans", "es-419", "ru", "ko-KR", "it", "pl", "uk", "unknown"}) {
            expect(make_activity(config, bilingual_status(language), 123).at("details") == "Exploring Blue Creek Apartments", "Non-Portuguese game languages normalize Steam's Portuguese text to English");
        }
        for (const auto& language : {"pt", "pt-BR", "pt-PT", "PT_br", "brazilian", "portuguese"}) {
            expect(make_activity(config, bilingual_status(language), 123).at("details") == u8"está na zona:Edifício Blue Creek", "Portuguese game languages keep the real Portuguese status");
            expect(make_activity(config, {"", "", language}, 123).at("details") == "Jogando SILENT HILL 2", "Portuguese fallback is localized");
        }
        const auto english_menu = steam_status_from_presence({{"steam_display", menu.details}, {"gamezone", "BluecreekApartments"}}, "en");
        expect(make_activity(config, english_menu, 123).at("details") == "Preparing to explore Silent Hill", "Menu text translates without retaining a stale zone");
        const auto french_region = steam_status_from_presence({{"steam_display", "Dans la zone : Appartements Blue Creek"}, {"gamezone", "BluecreekApartments"}}, "fr");
        expect(make_activity(config, french_region, 123).at("details") == "Exploring Blue Creek Apartments", "Zone IDs work independently of Steam's text language");
        expect(make_activity(config, {"2124490", "localized status", "de", "WoodsideApartmentsOtherworld"}, 123).at("details") == "Exploring Wood Side Apartments (Otherworld)", "Explicit Otherworld zone IDs are preserved");
        expect(make_activity(config, {"2124490", "localized status", "de", "#UnresolvedZone"}, 123).at("details") == "Playing SILENT HILL 2", "Unresolved zone tokens are never invented as locations");
        expect(make_activity(config, {"2124490", "localized status", "ja", u8"不明な場所"}, 123).at("details") == "Playing SILENT HILL 2", "Unknown localized zones use English fallback");
        expect(make_activity(config, {"2124490", "localized status", "de", "NewRegion"}, 123).at("details") == "Exploring New Region", "Readable English zone identifiers are split into words");
        const auto translated_rules = parse_config({{"image_rules", {{{"contains", {"blue creek"}}, {"image", "blue_creek"}}}}});
        expect(make_activity(translated_rules, french_region, 123).at("assets").at("large_image") == "blue_creek", "Artwork rules also match translated text");
        expect(make_activity(config, bilingual_status("en"), 123).at("timestamps") == make_activity(config, bilingual_status("pt-BR"), 123).at("timestamps"), "Language changes preserve the timer");
        std::size_t catalog_cases = 0;
        for (const auto& region : region_fixtures) {
            for (const auto* alias : region.aliases) {
                for (const auto* language : {"en", "fr", "ja", "pt-BR"}) {
                    const auto mapped = steam_status_from_presence({{"steam_display", "Localized region activity"}, {"gamezone", alias}}, language);
                    const auto activity = make_activity(config, mapped, 123);
                    const bool portuguese = std::string(language) == "pt-BR";
                    const auto expected = std::string(portuguese ? "Explorando " : "Exploring ") + (portuguese ? region.portuguese : region.english);
                    if (activity.at("details") != expected) throw std::runtime_error(std::string("Region catalog mismatch: ") + alias + " / " + language);
                    expect(activity.at("timestamps").at("start") == 123, "Every region keeps the session timer");
                    ++catalog_cases;
                }
            }
        }
        std::cout << "Region catalog: 15 regions; " << catalog_cases << " localized alias cases passed.\n";
        const auto mapped_region = [&](const char* zone) {
            return make_activity(config, {"2124490", "Localized region activity", "en", zone}, 123).at("details").get<std::string>();
        };
        expect(mapped_region("WesternSouthValeNightfall") != mapped_region("WesternSouthValeOtherworld"), "Nightfall and Otherworld must not be collapsed");
        expect(mapped_region("Labyrinth") != mapped_region("LabyrinthRottenArea"), "Labyrinth subareas must not collapse to the main area");
        expect(mapped_region("BrookhavenHospital") != mapped_region("OtherworldHospital"), "Normal and Otherworld hospital stay distinct");
        expect(mapped_region("LakeviewHotel") != mapped_region("LakeviewHotelOtherworld"), "Normal and Otherworld hotel stay distinct");
        expect(make_activity(config, {"2124490", "Exploring West City", "en"}, 123).at("details") == "Exploring Western South Vale", "West City activity text is standardized without a gamezone key");
        expect(make_activity(config, {"2124490", "is in zone:East City", "en"}, 123).at("details") == "Exploring Eastern South Vale", "East City activity text is standardized without a gamezone key");
        expect(mapped_region("WestCityNightfall") != mapped_region("WestCityOtherworld"), "City aliases preserve the Nightfall distinction");
        const auto specific_images = parse_config({{"image_rules", {{{"contains", {"labyrinth", "rotten area"}}, {"image", "labyrinth_rotten"}}, {{"contains", {"labyrinth"}}, {"image", "labyrinth"}}}}});
        expect(make_activity(specific_images, {"2124490", "Localized region activity", "de", "LabyrinthRottenArea"}, 123).at("assets").at("large_image") == "labyrinth_rotten", "Specific artwork matches translated Labyrinth subareas before general rules");
        expect(make_activity(config, steam_status_from_presence({{"gamezone", "BluecreekApartments"}}, "en"), 123).at("details") == "Playing SILENT HILL 2", "A zone without an active status cannot retain a stale location");
        expect(parse_profile_language(profile_fixture("pt-BR")) == "pt-BR", "Read the game's saved Portuguese setting");
        expect(parse_profile_language(profile_fixture("fr", true)) == "fr", "Read a UTF-16 language property");
        expect(parse_profile_language(profile_fixture("")) == "en", "The game omits Language when the default English setting is selected");
        expect(parse_profile_language(profile_fixture("../invalid")).empty(), "Reject invalid locale characters");
        expect(parse_profile_language("GVAS").empty(), "Incomplete profiles must not imply English");
        const auto valid_profile = profile_fixture("pt-BR");
        for (std::size_t length = 0; length < valid_profile.size(); ++length) {
            expect(parse_profile_language(valid_profile.substr(0, length)).empty(), "Truncated language profiles are ignored");
        }
        expect(steam_status_from_presence({{"steam_display", "Current location"}, {"status", "Old status"}}).details == "Current location", "Display field takes precedence");
        expect(steam_status_from_presence({{"steam_display", "#UnresolvedToken"}, {"status", "Exploring"}}).details == "Exploring", "Plain status fallback");
        expect(steam_status_from_presence({{"steam_display", "#UnresolvedToken"}}).details.empty(), "Never show raw localization tokens");
        expect(steam_status_from_presence({}).details.empty(), "Missing local data clears old region");
        expect(steam_status_from_presence({{"steam_display", "   "}}).details.empty(), "Whitespace is not a location");
        expect(parse_config({{"language", "brazilian"}, {"poll_interval_seconds", 15}}).poll_interval_seconds == 15, "Old settings remain loadable");
        expect(truncate_utf8(u8"á👻z", 2) == u8"á👻", "UTF-8 truncation preserves complete characters");
        bool invalid_config = false;
        try { parse_config({{"poll_interval_seconds", 0}}); } catch (const std::exception&) { invalid_config = true; }
        expect(invalid_config, "Rate limit enforced");
        invalid_config = false;
        try { parse_config({{"application_id", "not-an-id"}}); } catch (const std::exception&) { invalid_config = true; }
        expect(invalid_config, "Invalid application ID rejected");
        invalid_config = false;
        try { parse_config({{"discord_pipe", 20}}); } catch (const std::exception&) { invalid_config = true; }
        expect(invalid_config, "Invalid pipe index rejected");
        const auto malformed = parse_miniprofile("<span class='rich_presence'>" + std::string(65536, 'x'));
        expect(malformed.details.empty(), "Malformed oversized span handled");
        std::cout << "Core checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
