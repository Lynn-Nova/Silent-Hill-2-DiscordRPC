#pragma once

#include "core.hpp"
#include <windows.h>
#include <map>

namespace sh2presence {

SteamPresence fetch_local_steam_presence();
std::int64_t process_start_time();

class DiscordClient {
public:
    explicit DiscordClient(HANDLE stop);
    ~DiscordClient();
    DiscordClient(const DiscordClient&) = delete;
    DiscordClient& operator=(const DiscordClient&) = delete;
    void connect(const std::string& application_id, int pipe_index);
    void update(const nlohmann::json& activity);
    void pump();
    void close();
    void clear();
    bool connected() const;

private:
    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    HANDLE stop_ = nullptr;
    std::uint64_t nonce_ = 0;
    void transfer(void* data, DWORD size, bool writing, ULONGLONG deadline, bool interruptible = true);
    void send_frame(std::uint32_t opcode, const std::string& body, ULONGLONG deadline, bool interruptible = true);
    std::pair<std::uint32_t, std::string> read_frame(ULONGLONG deadline);
    nlohmann::json response(const std::string& nonce, bool handshake, ULONGLONG deadline);
};

}
