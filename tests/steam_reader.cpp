#include "windows.hpp"
#include "profile_fixture.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

void expect(bool condition, const char* description) {
    if (!condition) throw std::runtime_error(description);
}

bool reader_fails() {
    try { sh2presence::fetch_local_steam_presence(); } catch (const std::exception&) { return true; }
    return false;
}

int main(int argc, char** argv) {
    try {
        if (argc != 2) return 1;
        expect(reader_fails(), "Unloaded Steam API must be handled");
        const auto module = LoadLibraryA(argv[1]);
        expect(module != nullptr, "Cannot load the test Steam API");
        const auto set_mode = reinterpret_cast<void (*)(int)>(GetProcAddress(module, "TestSetMode"));
        expect(set_mode != nullptr, "Test export missing");
        expect(reader_fails(), "Do not access Steam interfaces before initialization");
        const auto read = [] {
            const auto presence = sh2presence::fetch_local_steam_presence();
            return sh2presence::steam_status_from_presence(presence.values, presence.game_language);
        };
        set_mode(1);
        const auto menu = read();
        expect(menu.game_language == "brazilian", "Steam language fallback when no local profile is available");
        expect(menu.details == u8"está se preparando para se aventurar em Silent Hill", "Local menu text");
        set_mode(2);
        const auto region = read();
        expect(region.details == u8"está na zona:Edifício Blue Creek", "Local region transition");
        const sh2presence::Config config;
        expect(sh2presence::make_activity(config, menu, 123).at("timestamps") == sh2presence::make_activity(config, region, 123).at("timestamps"), "Region transition must preserve the session timer");
        set_mode(3);
        expect(read().details == u8"Explorando Wood Side (Outro Mundo)", "Otherworld text preserved");
        set_mode(4);
        expect(read().details.empty(), "Cleared presence must clear old location");
        set_mode(5);
        expect(reader_fails(), "Unavailable interface must be handled");
        set_mode(6);
        expect(read().details == "Plain status", "Unresolved token uses plain status");
        set_mode(7);
        expect(read().details.empty(), "Null Steam strings must be handled");
        set_mode(8);
        expect(reader_fails(), "Invalid Steam key count must be handled");
        set_mode(9);
        const auto translated = read();
        expect(translated.game_language == "french", "Non-Portuguese Steam language detection");
        expect(sh2presence::make_activity(config, translated, 123).at("details") == "Exploring Blue Creek Apartments", "Local Steam zone translates into English");
        wchar_t library_path[32768]{};
        expect(GetModuleFileNameW(module, library_path, 32768) != 0, "Cannot locate the test library");
        const auto test_data = std::filesystem::path(library_path).parent_path() / "test-localappdata";
        const auto profile = test_data / "SilentHill2" / "Saved" / "SaveGames" / "1" / "PlayerProfile.sav";
        std::filesystem::create_directories(profile.parent_path());
        expect(SetEnvironmentVariableW(L"LOCALAPPDATA", test_data.c_str()) != 0, "Cannot isolate the test profile directory");
        const auto write_profile = [&](const std::string& bytes) {
            std::ofstream file(profile, std::ios::binary | std::ios::trunc);
            file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
            expect(static_cast<bool>(file), "Cannot write a temporary test fixture");
        };
        write_profile(profile_fixture("pt-BR"));
        expect(read().game_language == "pt-BR", "Actual game profile overrides Steam's different language");
        write_profile(profile_fixture(""));
        expect(read().game_language == "en", "Default English is detected even when Steam uses French");
        write_profile(profile_fixture("de", true));
        expect(read().game_language == "de", "A game language change is detected without restarting the plugin");
        write_profile("GVAS");
        expect(read().game_language == "french", "Unreadable game profiles fall back to Steam's language");
        FreeLibrary(module);
        expect(reader_fails(), "Unloaded Steam API must not leave stale function pointers");
        std::cout << "Local Steam API checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
