#include "core.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <stdexcept>

namespace sh2presence {
namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool portuguese_language(std::string language) {
    language = lowercase(std::move(language));
    std::replace(language.begin(), language.end(), '_', '-');
    return language == "pt" || language.rfind("pt-", 0) == 0 || language == "brazilian" || language == "portuguese";
}

bool portuguese_text(const std::string& text) {
    const auto comparable = lowercase(text);
    return comparable.rfind(u8"está na zona:", 0) == 0 || comparable.rfind(u8"está se preparando", 0) == 0 || comparable.rfind("explorando ", 0) == 0 || comparable.rfind("a explorar ", 0) == 0 || comparable.rfind("jogando ", 0) == 0 || comparable.rfind("no menu", 0) == 0;
}

std::string zone_name(const std::string& source, bool portuguese) {
    if (source.empty() || source.size() > 80) return {};
    std::string identifier;
    for (unsigned char character : source) {
        if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9')) identifier += static_cast<char>(std::tolower(character));
        else if (character != ' ' && character != '_' && character != '-' && character != ':' && character != '(' && character != ')') return {};
    }
    if (identifier.empty() || identifier == "none" || identifier == "unknown" || identifier == "unknownzone") return {};
    struct Name {
        const char* english;
        const char* portuguese;
        std::initializer_list<const char*> identifiers;
    };
    static const Name names[] = {
        {"Eastern South Vale", "Leste de South Vale", {"easternsouthvale", "eastsouthvale", "eastcity"}},
        {"Wood Side Apartments", u8"Edifício Wood Side", {"woodsideapartments"}},
        {"Blue Creek Apartments", u8"Edifício Blue Creek", {"bluecreekapartments"}},
        {"Western South Vale", "Oeste de South Vale", {"westernsouthvale", "westsouthvale", "westcity"}},
        {"Brookhaven Hospital", "Hospital Brookhaven", {"brookhavenhospital"}},
        {"Otherworld Hospital", "Hospital Brookhaven: Outro Mundo", {"otherworldhospital", "hospitalotherworld", "brookhavenhospitalotherworld", "otherworldbrookhavenhospital", "brookhavenhospitalow", "hospitalow"}},
        {"Eastern South Vale: Otherworld", "Leste de South Vale: Outro Mundo", {"easternsouthvaleotherworld", "eastsouthvaleotherworld", "otherworldeasternsouthvale", "otherworldeastsouthvale", "easternsouthvaleow", "eastsouthvaleow", "eastcityotherworld", "otherworldeastcity", "eastcityow"}},
        {"Western South Vale: Nightfall", "Oeste de South Vale: Anoitecer", {"westernsouthvalenightfall", "westsouthvalenightfall", "nightfallwesternsouthvale", "nightfallwestsouthvale", "westcitynightfall"}},
        {"Toluca Prison", u8"Prisão Toluca", {"tolucaprison"}},
        {"Labyrinth", "Labirinto", {"labyrinth"}},
        {"Labyrinth: Rotten Area", u8"Labirinto: Área Apodrecida", {"labyrinthrottenarea", "labyrinthrotten", "rottenarealabyrinth", "rottenarea"}},
        {"Labyrinth: Desolate Area", u8"Labirinto: Área Desolada", {"labyrinthdesolatearea", "labyrinthdesolate", "desolatearealabyrinth", "desolatearea"}},
        {"Labyrinth: Ruined Area", u8"Labirinto: Área em Ruínas", {"labyrinthruinedarea", "labyrinthruined", "ruinedarealabyrinth", "ruinedarea"}},
        {"Lakeview Hotel", "Hotel Lakeview", {"lakeviewhotel"}},
        {"Lakeview Hotel: Otherworld", "Hotel Lakeview: Outro Mundo", {"lakeviewhotelotherworld", "otherworldlakeviewhotel", "lakeviewhotelow"}},
        {"Toluca Lake", "Lago Toluca", {"tolucalake"}},
        {"South Vale", "South Vale", {"southvale"}},
        {"Silent Hill Historical Society", u8"Sociedade Histórica de Silent Hill", {"historicalsociety", "silenthillhistoricalsociety"}},
        {"Rosewater Park", "Parque Rosewater", {"rosewaterpark"}},
        {"Observation Deck", "Mirante", {"observationdeck"}},
        {"Forest", "Floresta", {"forest"}}
    };
    const auto lookup = [&](const std::string& key) -> std::string {
        for (const auto& name : names) {
            if (std::any_of(name.identifiers.begin(), name.identifiers.end(), [&](const char* alias) { return key == alias; })) {
                return portuguese ? name.portuguese : name.english;
            }
        }
        return {};
    };
    auto result = lookup(identifier);
    if (!result.empty()) return result;
    bool otherworld = false;
    if (identifier.size() > 10 && identifier.compare(identifier.size() - 10, 10, "otherworld") == 0) {
        identifier.resize(identifier.size() - 10);
        otherworld = true;
    } else if (source.size() > 3 && lowercase(source.substr(source.size() - 3)) == "_ow") {
        identifier.resize(identifier.size() - 2);
        otherworld = true;
    }
    if (otherworld) result = lookup(identifier);
    if (result.empty()) {
        if (otherworld) return {};
        for (std::size_t index = 0; index < source.size(); ++index) {
            const auto character = static_cast<unsigned char>(source[index]);
            if (character == '_' || character == '-') {
                if (!result.empty() && result.back() != ' ') result += ' ';
            } else {
                if (index && character >= 'A' && character <= 'Z' && source[index - 1] >= 'a' && source[index - 1] <= 'z') result += ' ';
                result += static_cast<char>(character);
            }
        }
        while (!result.empty() && result.back() == ' ') result.pop_back();
    }
    if (otherworld) result += portuguese ? " (Outro Mundo)" : " (Otherworld)";
    return result;
}

std::string activity_details(const SteamStatus& status) {
    const bool portuguese = portuguese_language(status.game_language) || (status.game_language.empty() && portuguese_text(status.details));
    const std::string fallback = portuguese ? "Jogando SILENT HILL 2" : "Playing SILENT HILL 2";
    if (status.game_id != "2124490" || status.details.empty()) return fallback;
    const auto comparable = lowercase(status.details);
    if (portuguese && portuguese_text(status.details)) return status.details;
    const bool menu = comparable.find("se preparando") != std::string::npos || comparable.find("main menu") != std::string::npos || comparable.find("preparing to") != std::string::npos || comparable.find("getting ready") != std::string::npos;
    if (menu) return portuguese ? "No menu principal" : "Preparing to explore Silent Hill";
    const auto zone = zone_name(status.zone, portuguese);
    if (!zone.empty()) return std::string(portuguese ? "Explorando " : "Exploring ") + zone;
    if (!portuguese) {
        for (const auto* prefix : {"exploring ", "is in zone:"}) {
            const std::string label(prefix);
            if (comparable.rfind(label, 0) != 0) continue;
            const auto named_zone = zone_name(status.details.substr(label.size()), false);
            if (!named_zone.empty()) return "Exploring " + named_zone;
        }
    }
    if (!portuguese && (comparable.rfind("exploring ", 0) == 0 || comparable.rfind("playing ", 0) == 0 || comparable.rfind("is in zone:", 0) == 0)) return status.details;
    return fallback;
}

std::string attribute(const std::string& tag, const std::string& name) {
    std::size_t position = 0;
    while ((position = tag.find('=', position)) != std::string::npos) {
        auto name_end = position;
        while (name_end && std::isspace(static_cast<unsigned char>(tag[name_end - 1]))) --name_end;
        auto name_start = name_end;
        while (name_start && (std::isalnum(static_cast<unsigned char>(tag[name_start - 1])) || tag[name_start - 1] == '-' || tag[name_start - 1] == '_')) --name_start;
        auto value_start = position + 1;
        while (value_start < tag.size() && std::isspace(static_cast<unsigned char>(tag[value_start]))) ++value_start;
        if (value_start == tag.size()) break;
        const auto quote = tag[value_start];
        if (quote != '\'' && quote != '"') { position = value_start + 1; continue; }
        const auto value_end = tag.find(quote, value_start + 1);
        if (value_end == std::string::npos) break;
        if (lowercase(tag.substr(name_start, name_end - name_start)) == name) {
            return tag.substr(value_start + 1, value_end - value_start - 1);
        }
        position = value_end + 1;
    }
    return {};
}

bool has_class(const std::string& tag, const std::string& target) {
    std::istringstream classes(attribute(tag, "class"));
    std::string token;
    while (classes >> token) if (token == target) return true;
    return false;
}

std::string encode_utf8(std::uint32_t codepoint) {
    std::string result;
    if (codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff) || codepoint == 0) {
        return "\xef\xbf\xbd";
    }
    if (codepoint < 0x80) {
        result += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        result += static_cast<char>(0xc0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3f));
    } else if (codepoint < 0x10000) {
        result += static_cast<char>(0xe0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f));
        result += static_cast<char>(0x80 | (codepoint & 0x3f));
    } else {
        result += static_cast<char>(0xf0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f));
        result += static_cast<char>(0x80 | (codepoint & 0x3f));
    }
    return result;
}

std::string decode_entities(const std::string& input) {
    std::string output;
    std::size_t previous = 0;
    std::size_t position = 0;
    while ((position = input.find('&', previous)) != std::string::npos) {
        output.append(input, previous, position - previous);
        const auto ending = input.find(';', position + 1);
        if (ending == std::string::npos || ending - position > 16) {
            output += '&';
            previous = position + 1;
            continue;
        }
        const auto name = input.substr(position + 1, ending - position - 1);
        if (name == "amp") output += '&';
        else if (name == "lt") output += '<';
        else if (name == "gt") output += '>';
        else if (name == "quot") output += '"';
        else if (name == "apos") output += '\'';
        else if (name == "nbsp") output += ' ';
        else if (name.size() > 1 && name[0] == '#') {
            try {
                const bool hexadecimal = name.size() > 2 && name[1] == 'x';
                const auto digits = name.substr(hexadecimal ? 2 : 1);
                std::size_t consumed = 0;
                const auto number = std::stoul(digits, &consumed, hexadecimal ? 16 : 10);
                if (consumed != digits.size()) throw std::runtime_error("Invalid HTML entity.");
                output += encode_utf8(number > 0x10ffff ? 0xfffd : static_cast<std::uint32_t>(number));
            } catch (const std::exception&) {
                output += "\xef\xbf\xbd";
            }
        } else output.append(input, position, ending - position + 1);
        previous = ending + 1;
    }
    output.append(input, previous, std::string::npos);
    return output;
}

std::string text_content(const std::string& html) {
    std::string stripped;
    bool inside_tag = false;
    for (const auto character : html) {
        if (character == '<') inside_tag = true;
        else if (character == '>') inside_tag = false;
        else if (!inside_tag) stripped += character;
    }
    const auto decoded = decode_entities(stripped);
    std::string output;
    bool pending_space = false;
    for (unsigned char character : decoded) {
        if (std::isspace(character)) {
            pending_space = !output.empty();
        } else {
            if (pending_space) output += ' ';
            pending_space = false;
            output += static_cast<char>(character);
        }
    }
    return output;
}

}

Config parse_config(const nlohmann::json& input) {
    if (!input.is_object()) throw std::runtime_error("Configuration must be a JSON object.");
    Config config;
    config.enabled = input.value("enabled", config.enabled);
    config.application_id = input.value("application_id", config.application_id);
    config.default_image = input.value("default_image", config.default_image);
    config.poll_interval_seconds = input.value("poll_interval_seconds", config.poll_interval_seconds);
    config.discord_pipe = input.value("discord_pipe", config.discord_pipe);
    if (config.application_id.size() < 17 || config.application_id.size() > 20 || !std::all_of(config.application_id.begin(), config.application_id.end(), [](char character) { return character >= '0' && character <= '9'; })) {
        throw std::runtime_error("application_id must contain 17 to 20 digits.");
    }
    if (config.poll_interval_seconds < 1 || config.poll_interval_seconds > 120) {
        throw std::runtime_error("poll_interval_seconds must be between 1 and 120.");
    }
    if (config.discord_pipe < -1 || config.discord_pipe > 9) {
        throw std::runtime_error("discord_pipe must be -1 (automatic) or 0 to 9.");
    }
    if (config.default_image.empty() || config.default_image.size() > 512) {
        throw std::runtime_error("default_image must have 1 to 512 bytes.");
    }
    if (input.contains("image_rules")) {
        const auto& rules = input.at("image_rules");
        if (!rules.is_array() || rules.size() > 64) throw std::runtime_error("image_rules must be an array of at most 64 rules.");
        for (const auto& rule : rules) {
            ImageRule parsed{rule.at("contains").get<std::vector<std::string>>(), rule.at("image").get<std::string>()};
            if (parsed.contains.empty() || parsed.contains.size() > 16 || parsed.image.empty() || parsed.image.size() > 512) {
                throw std::runtime_error("Each image rule needs 1 to 16 expressions and an image.");
            }
            for (const auto& expression : parsed.contains) {
                if (expression.empty() || expression.size() > 256) throw std::runtime_error("Invalid image rule expression.");
            }
            config.image_rules.push_back(std::move(parsed));
        }
    }
    return config;
}

Config load_config(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return Config{};
    if (std::filesystem::file_size(path) > 65536) throw std::runtime_error("Configuration is larger than 64 KiB.");
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open configuration.");
    return parse_config(nlohmann::json::parse(file));
}

SteamStatus parse_miniprofile(const std::string& html) {
    SteamStatus status;
    const auto comparable = lowercase(html);
    std::size_t position = 0;
    while ((position = comparable.find("<img", position)) != std::string::npos) {
        const auto ending = html.find('>', position);
        if (ending == std::string::npos) break;
        const auto tag = html.substr(position, ending - position + 1);
        position = ending + 1;
        if (!has_class(tag, "game_logo")) continue;
        const auto source = attribute(tag, "src");
        const auto apps = source.find("/apps/");
        if (apps == std::string::npos) continue;
        const auto first = apps + 6;
        const auto last = source.find_first_not_of("0123456789", first);
        if (last != std::string::npos && last > first && source[last] == '/') status.game_id = source.substr(first, last - first);
    }
    position = 0;
    while ((position = comparable.find("<span", position)) != std::string::npos) {
        const auto ending = html.find('>', position);
        if (ending == std::string::npos) break;
        const auto tag = html.substr(position, ending - position + 1);
        position = ending + 1;
        if (!has_class(tag, "rich_presence")) continue;
        auto cursor = position;
        int depth = 1;
        while (depth) {
            const auto closing = comparable.find("</span", cursor);
            if (closing == std::string::npos || closing - position > 8192) return status;
            const auto opening = comparable.find("<span", cursor);
            if (opening != std::string::npos && opening < closing) {
                ++depth;
                cursor = opening + 5;
            } else {
                --depth;
                if (!depth) { status.details = text_content(html.substr(position, closing - position)); return status; }
                cursor = closing + 6;
            }
        }
    }
    return status;
}

std::string parse_profile_language(const std::string& data) {
    if (data.size() > 1048576 || data.compare(0, 4, "GVAS") != 0) return {};
    const auto profile_class = std::string("/Script/SHProto.SHPlayerProfile") + '\0';
    const std::string ending("\x05\0\0\0None\0\0\0\0\0", 13);
    if (data.find(profile_class) == std::string::npos || data.size() < ending.size() || data.compare(data.size() - ending.size(), ending.size(), ending) != 0) return {};
    const std::string marker("\x09\0\0\0Language\0\x0c\0\0\0StrProperty\0", 29);
    auto position = data.find(marker);
    if (position == std::string::npos) return data.find(std::string("Language\0", 9)) == std::string::npos ? "en" : "";
    position += marker.size();
    const auto number = [&](std::size_t offset) -> std::uint32_t {
        std::uint32_t value = 0;
        for (std::size_t index = 0; index < 4; ++index) value |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[offset + index])) << (index * 8);
        return value;
    };
    if (data.size() - position < 13) return {};
    const auto payload_size = number(position);
    if (payload_size < 6 || payload_size > 72 || number(position + 4) != 0) return {};
    const auto has_guid = static_cast<unsigned char>(data[position + 8]);
    if (has_guid > 1) return {};
    position += 9 + (has_guid ? 16 : 0);
    if (position > data.size() || data.size() - position < payload_size) return {};
    const auto encoded_length = static_cast<std::int32_t>(number(position));
    const bool wide = encoded_length < 0;
    if (encoded_length < -33 || encoded_length > 33 || encoded_length == 0) return {};
    const auto length = static_cast<std::size_t>(wide ? -encoded_length : encoded_length);
    const auto width = wide ? 2u : 1u;
    if (length < 3 || payload_size != 4 + length * width) return {};
    position += 4;
    std::string language;
    for (std::size_t index = 0; index < length; ++index) {
        const auto character = static_cast<unsigned char>(data[position + index * width]);
        if (wide && data[position + index * width + 1] != '\0') return {};
        if (index + 1 == length) {
            if (character != 0) return {};
        } else {
            if (!((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9') || character == '-' || character == '_')) return {};
            language += static_cast<char>(character);
        }
    }
    return language;
}

SteamStatus steam_status_from_presence(const std::map<std::string, std::string>& presence, const std::string& game_language) {
    SteamStatus status;
    status.game_language = game_language;
    for (const auto* key : {"steam_display", "status"}) {
        const auto found = presence.find(key);
        if (found == presence.end()) continue;
        const auto& text = found->second;
        const auto start = text.find_first_not_of(" \t\r\n");
        if (start != std::string::npos && text[start] != '#' && text.size() <= 256) {
            status.game_id = "2124490";
            status.details = text;
            const auto zone = presence.find("gamezone");
            if (zone != presence.end()) status.zone = zone->second;
            return status;
        }
    }
    return status;
}

std::string truncate_utf8(const std::string& value, std::size_t characters) {
    std::size_t index = 0;
    std::size_t count = 0;
    while (index < value.size()) {
        if ((static_cast<unsigned char>(value[index]) & 0xc0) != 0x80) {
            if (count == characters) break;
            ++count;
        }
        ++index;
    }
    return value.substr(0, index);
}

nlohmann::json make_activity(const Config& config, const SteamStatus& status, std::int64_t started) {
    const bool has_details = status.game_id == "2124490" && !status.details.empty();
    const auto details = activity_details(status);
    auto image = config.default_image;
    if (has_details) {
        const auto comparable = lowercase(status.details);
        const auto translated = lowercase(details);
        for (const auto& rule : config.image_rules) {
            if (std::all_of(rule.contains.begin(), rule.contains.end(), [&](const auto& term) {
                return comparable.find(lowercase(term)) != std::string::npos;
            }) || std::all_of(rule.contains.begin(), rule.contains.end(), [&](const auto& term) {
                return translated.find(lowercase(term)) != std::string::npos;
            })) {
                image = rule.image;
                break;
            }
        }
    }
    return {{"name", "SILENT HILL 2"}, {"type", 0}, {"details", truncate_utf8(details, 128)},
            {"timestamps", {{"start", started}}}, {"assets", {{"large_image", image}, {"large_text", "SILENT HILL 2"}}}};
}

}
