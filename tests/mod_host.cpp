#include <windows.h>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const bool loader_mode = argc == 3 && std::string(argv[1]) == "--loader";
    if (argc != 2 && !loader_mode) return 1;
    HMODULE plugin = LoadLibraryA(argv[loader_mode ? 2 : 1]);
    if (!plugin) { std::cerr << "LoadLibrary failed: " << GetLastError() << '\n'; return 1; }
    if (loader_mode) {
        Sleep(750);
        plugin = GetModuleHandleA("SH2DiscordPresence.asi");
        if (!plugin) { std::cerr << "Loader did not load the ASI plugin.\n"; return 1; }
    }
    const auto initialize = reinterpret_cast<void(*)()>(GetProcAddress(plugin, "InitializeASI"));
    const auto shutdown = reinterpret_cast<void(*)()>(GetProcAddress(plugin, "ShutdownASI"));
    if (!initialize || !shutdown) return 1;
    initialize();
    initialize();
    Sleep(750);
    shutdown();
    shutdown();
    FreeLibrary(plugin);
    std::cout << "Native mod initialization and shutdown passed.\n";
    return 0;
}
