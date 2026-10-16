#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace sh2presence {

struct ImageRule {
    std::vector<std::string> contains;
    std::string image;
};

struct Config {
    bool enabled = true;
    std::string application_id = "1560708389796061245";
    std::string default_image = "https://shared.fastly.steamstatic.com/store_item_assets/steam/apps/2124490/header.jpg";
    int poll_interval_seconds = 2;
    int discord_pipe = -1;
    std::vector<ImageRule> image_rules;
};

struct SteamStatus {
    std::string game_id;
    std::string details;
    std::string game_language;
    std::string zone;
};

struct SteamPresence {
    std::map<std::string, std::string> values;
    std::string game_language;
};

Config parse_config(const nlohmann::json& input);
Config load_config(const std::filesystem::path& path);
SteamStatus parse_miniprofile(const std::string& html);
SteamStatus steam_status_from_presence(const std::map<std::string, std::string>& presence, const std::string& game_language = {});
std::string parse_profile_language(const std::string& data);
std::string truncate_utf8(const std::string& value, std::size_t characters);
nlohmann::json make_activity(const Config& config, const SteamStatus& status, std::int64_t started);

}
