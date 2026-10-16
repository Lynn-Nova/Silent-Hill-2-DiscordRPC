#pragma once

#include <cstdint>
#include <string>

inline std::string profile_fixture(const std::string& language, bool wide = false) {
    std::string bytes = "GVAS";
    bytes += std::string("/Script/SHProto.SHPlayerProfile") + '\0';
    const auto append_number = [&](std::uint32_t value) {
        for (unsigned index = 0; index < 4; ++index) bytes += static_cast<char>((value >> (index * 8)) & 0xff);
    };
    if (!language.empty()) {
        bytes.append("\x09\0\0\0Language\0\x0c\0\0\0StrProperty\0", 29);
        const auto length = static_cast<std::uint32_t>(language.size() + 1);
        append_number(4 + length * (wide ? 2 : 1));
        append_number(0);
        bytes += '\0';
        append_number(wide ? static_cast<std::uint32_t>(-static_cast<std::int32_t>(length)) : length);
        for (char character : language) {
            bytes += character;
            if (wide) bytes += '\0';
        }
        bytes += '\0';
        if (wide) bytes += '\0';
    }
    bytes.append("\x05\0\0\0None\0\0\0\0\0", 13);
    return bytes;
}
