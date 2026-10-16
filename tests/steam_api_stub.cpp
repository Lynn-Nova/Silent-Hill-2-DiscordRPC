#include <cstdint>
#include <cstring>

namespace {
int mode = 0;
int interface_marker = 0;
}

extern "C" __declspec(dllexport) void TestSetMode(int value) { mode = value; }
extern "C" __declspec(dllexport) int SteamAPI_GetHSteamUser() { return mode == 0 ? 0 : 1; }
extern "C" __declspec(dllexport) void* SteamAPI_SteamFriends_v017() { return mode == 5 ? nullptr : &interface_marker; }
extern "C" __declspec(dllexport) void* SteamAPI_SteamUser_v021() { return &interface_marker; }
extern "C" __declspec(dllexport) void* SteamAPI_SteamApps_v008() { return &interface_marker; }
extern "C" __declspec(dllexport) const char* SteamAPI_ISteamApps_GetCurrentGameLanguage(void*) { return mode == 9 ? "french" : "brazilian"; }
extern "C" __declspec(dllexport) std::uint64_t SteamAPI_ISteamUser_GetSteamID(void*) { return 1; }
extern "C" __declspec(dllexport) int SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount(void*, std::uint64_t) { return mode == 8 ? 31 : 3; }
extern "C" __declspec(dllexport) const char* SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex(void*, std::uint64_t, int index) { return index == 0 ? "steam_display" : index == 1 ? "status" : "gamezone"; }

extern "C" __declspec(dllexport) const char* SteamAPI_ISteamFriends_GetFriendRichPresence(void* instance, std::uint64_t account, const char* key) {
    if (instance != &interface_marker || account != 1) return "";
    if (std::strcmp(key, "gamezone") == 0) return mode == 9 ? "BluecreekApartments" : "";
    if (std::strcmp(key, "status") == 0) return mode == 6 ? "Plain status" : "";
    if (std::strcmp(key, "steam_display") != 0) return "";
    if (mode == 1) return u8"está se preparando para se aventurar em Silent Hill";
    if (mode == 2 || mode == 9) return u8"está na zona:Edifício Blue Creek";
    if (mode == 3) return u8"Explorando Wood Side (Outro Mundo)";
    if (mode == 6) return "#UnresolvedToken";
    if (mode == 7) return nullptr;
    return "";
}
