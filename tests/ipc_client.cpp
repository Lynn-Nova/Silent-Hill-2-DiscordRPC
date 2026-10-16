#include "windows.hpp"
#include <iostream>

int main(int argc, char** argv) {
    const bool expect_failure = argc > 1 && std::string(argv[1]) == "failure";
    HANDLE stop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    int result = 0;
    try {
        sh2presence::DiscordClient client(stop);
        client.connect("1560708389796061245", 9);
        client.update(sh2presence::make_activity({}, {"2124490", u8"Explorando Wood Side — Outro Mundo"}, 1700000000));
        client.clear();
        if (expect_failure) throw std::logic_error("Expected an IPC failure.");
        std::cout << "IPC update and cleanup passed.\n";
    } catch (const std::logic_error& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    } catch (const std::exception& error) {
        std::cout << error.what() << '\n';
        result = expect_failure ? 0 : 1;
    }
    CloseHandle(stop);
    return result;
}
