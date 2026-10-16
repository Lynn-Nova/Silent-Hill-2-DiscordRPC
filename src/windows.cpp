#include "windows.hpp"

#include <array>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace sh2presence {
namespace {

void validate_rpc(const nlohmann::json& payload) {
    if (payload.value("evt", nlohmann::json{}) == "ERROR" || payload.contains("code")) {
        throw std::runtime_error("Discord rejected the request: " + payload.dump().substr(0, 512));
    }
}

std::string profile_language(std::uint64_t account) {
    wchar_t directory[32768]{};
    const auto length = GetEnvironmentVariableW(L"LOCALAPPDATA", directory, 32768);
    if (!length || length >= 32768) return {};
    const auto path = std::filesystem::path(directory) / L"SilentHill2" / L"Saved" / L"SaveGames" / std::to_wstring(account) / L"PlayerProfile.sav";
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 1048576) return {};
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::string bytes(static_cast<std::size_t>(size), '\0');
    if (!file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))) return {};
    return parse_profile_language(bytes);
}

}

SteamPresence fetch_local_steam_presence() {
    const auto steam = GetModuleHandleW(L"steam_api64.dll");
    if (!steam) throw std::runtime_error("The game's Steam API has not loaded yet.");
    const auto current_user = reinterpret_cast<int (*)()>(GetProcAddress(steam, "SteamAPI_GetHSteamUser"));
    const auto friends_accessor = reinterpret_cast<void* (*)()>(GetProcAddress(steam, "SteamAPI_SteamFriends_v017"));
    const auto user_accessor = reinterpret_cast<void* (*)()>(GetProcAddress(steam, "SteamAPI_SteamUser_v021"));
    const auto user_id = reinterpret_cast<std::uint64_t (*)(void*)>(GetProcAddress(steam, "SteamAPI_ISteamUser_GetSteamID"));
    const auto key_count = reinterpret_cast<int (*)(void*, std::uint64_t)>(GetProcAddress(steam, "SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount"));
    const auto key_at = reinterpret_cast<const char* (*)(void*, std::uint64_t, int)>(GetProcAddress(steam, "SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex"));
    const auto value_at = reinterpret_cast<const char* (*)(void*, std::uint64_t, const char*)>(GetProcAddress(steam, "SteamAPI_ISteamFriends_GetFriendRichPresence"));
    if (!current_user || !friends_accessor || !user_accessor || !user_id || !key_count || !key_at || !value_at) {
        throw std::runtime_error("The game's Steam API does not expose the expected read-only presence functions.");
    }
    if (!current_user()) throw std::runtime_error("The game has not initialized Steam yet.");
    auto* friends = friends_accessor();
    auto* user = user_accessor();
    if (!friends || !user) throw std::runtime_error("Steam interfaces are not ready.");
    const auto account = user_id(user);
    if (!account) throw std::runtime_error("Steam has no active local user.");
    const auto count = key_count(friends, account);
    if (count < 0 || count > 30) throw std::runtime_error("Unexpected Steam presence key count.");
    SteamPresence presence;
    presence.game_language = profile_language(account);
    if (presence.game_language.empty()) {
        const auto apps_get = reinterpret_cast<void* (*)()>(GetProcAddress(steam, "SteamAPI_SteamApps_v008"));
        const auto language_get = reinterpret_cast<const char* (*)(void*)>(GetProcAddress(steam, "SteamAPI_ISteamApps_GetCurrentGameLanguage"));
        if (apps_get && language_get) {
            if (auto* apps = apps_get()) {
                const auto* language = language_get(apps);
                if (language && strnlen(language, 33) <= 32) presence.game_language = language;
            }
        }
    }
    for (int index = 0; index < count; ++index) {
        const auto* raw_key = key_at(friends, account, index);
        if (!raw_key || strnlen(raw_key, 65) > 64) continue;
        const std::string key(raw_key);
        const auto* raw_value = value_at(friends, account, key.c_str());
        if (raw_value && strnlen(raw_value, 257) <= 256) presence.values.emplace(key, raw_value);
    }
    return presence;
}

std::int64_t process_start_time() {
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user)) {
        throw std::runtime_error("Cannot read the game session start time.");
    }
    ULARGE_INTEGER ticks{};
    ticks.LowPart = created.dwLowDateTime;
    ticks.HighPart = created.dwHighDateTime;
    return static_cast<std::int64_t>(ticks.QuadPart / 10000000ULL - 11644473600ULL);
}

DiscordClient::DiscordClient(HANDLE stop) : stop_(stop) {}
DiscordClient::~DiscordClient() { close(); }
bool DiscordClient::connected() const { return pipe_ != INVALID_HANDLE_VALUE; }

void DiscordClient::close() {
    if (connected()) {
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }
}

void DiscordClient::transfer(void* data, DWORD size, bool writing, ULONGLONG deadline, bool interruptible) {
    auto* bytes = static_cast<char*>(data);
    DWORD completed = 0;
    while (completed < size) {
        if (GetTickCount64() >= deadline) throw std::runtime_error("Discord IPC timed out.");
        OVERLAPPED operation{};
        operation.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!operation.hEvent) throw std::runtime_error("Cannot allocate an IPC event.");
        DWORD transferred = 0;
        BOOL success = writing ? WriteFile(pipe_, bytes + completed, size - completed, &transferred, &operation)
                               : ReadFile(pipe_, bytes + completed, size - completed, &transferred, &operation);
        DWORD error = success ? ERROR_SUCCESS : GetLastError();
        if (!success && error == ERROR_IO_PENDING) {
            HANDLE events[] = {operation.hEvent, stop_};
            const auto now = GetTickCount64();
            const DWORD remaining = now >= deadline ? 0 : static_cast<DWORD>(deadline - now);
            const DWORD count = interruptible && stop_ ? 2 : 1;
            const auto wait = WaitForMultipleObjects(count, events, FALSE, remaining);
            if (wait != WAIT_OBJECT_0) {
                CancelIoEx(pipe_, &operation);
                GetOverlappedResult(pipe_, &operation, &transferred, TRUE);
                CloseHandle(operation.hEvent);
                throw std::runtime_error("Discord IPC interrupted or timed out.");
            }
            success = GetOverlappedResult(pipe_, &operation, &transferred, FALSE);
            error = success ? ERROR_SUCCESS : GetLastError();
        }
        CloseHandle(operation.hEvent);
        if (!success || !transferred) throw std::runtime_error("Discord IPC closed (error " + std::to_string(error) + ").");
        completed += transferred;
    }
}

void DiscordClient::send_frame(std::uint32_t opcode, const std::string& body, ULONGLONG deadline, bool interruptible) {
    if (body.size() > 262144) throw std::runtime_error("Discord frame is too large.");
    std::array<std::uint32_t, 2> header{opcode, static_cast<std::uint32_t>(body.size())};
    transfer(header.data(), sizeof(header), true, deadline, interruptible);
    if (!body.empty()) transfer(const_cast<char*>(body.data()), static_cast<DWORD>(body.size()), true, deadline, interruptible);
}

std::pair<std::uint32_t, std::string> DiscordClient::read_frame(ULONGLONG deadline) {
    std::array<std::uint32_t, 2> header{};
    transfer(header.data(), sizeof(header), false, deadline);
    if (header[1] > 262144) throw std::runtime_error("Discord sent an oversized frame.");
    std::string body(header[1], '\0');
    if (!body.empty()) transfer(body.data(), header[1], false, deadline);
    return {header[0], std::move(body)};
}

nlohmann::json DiscordClient::response(const std::string& nonce, bool handshake, ULONGLONG deadline) {
    while (GetTickCount64() < deadline) {
        const auto [opcode, body] = read_frame(deadline);
        if (opcode == 2) throw std::runtime_error("Discord closed the connection.");
        if (opcode == 3) {
            send_frame(4, body, deadline);
            continue;
        }
        if (opcode != 1) throw std::runtime_error("Unexpected Discord IPC opcode.");
        const auto payload = nlohmann::json::parse(body);
        validate_rpc(payload);
        if (handshake && payload.value("evt", nlohmann::json{}) == "READY") return payload;
        if (!handshake && payload.value("nonce", nlohmann::json{}) == nonce && payload.value("cmd", "") == "SET_ACTIVITY") return payload;
    }
    throw std::runtime_error("Discord did not confirm the activity.");
}

void DiscordClient::connect(const std::string& application_id, int pipe_index) {
    close();
    const int first = pipe_index < 0 ? 0 : pipe_index;
    const int last = pipe_index < 0 ? 9 : pipe_index;
    for (int index = first; index <= last; ++index) {
        const auto path = L"\\\\.\\pipe\\discord-ipc-" + std::to_wstring(index);
        pipe_ = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if (connected()) break;
    }
    if (!connected()) throw std::runtime_error("Discord desktop is not available.");
    try {
        const auto deadline = GetTickCount64() + 5000;
        send_frame(0, nlohmann::json{{"v", 1}, {"client_id", application_id}}.dump(), deadline);
        response("", true, deadline);
    } catch (...) {
        close();
        throw;
    }
}

void DiscordClient::update(const nlohmann::json& activity) {
    const auto nonce = std::to_string(GetTickCount64()) + "-" + std::to_string(++nonce_);
    const auto deadline = GetTickCount64() + 5000;
    const nlohmann::json command{{"cmd", "SET_ACTIVITY"}, {"args", {{"pid", GetCurrentProcessId()}, {"activity", activity}}}, {"nonce", nonce}};
    send_frame(1, command.dump(), deadline);
    response(nonce, false, deadline);
}

void DiscordClient::pump() {
    DWORD available = 0;
    if (!PeekNamedPipe(pipe_, nullptr, 0, nullptr, &available, nullptr)) throw std::runtime_error("Discord disconnected.");
    if (available) {
        const auto deadline = GetTickCount64() + 1000;
        const auto [opcode, body] = read_frame(deadline);
        if (opcode == 3) send_frame(4, body, deadline);
        else if (opcode == 2) throw std::runtime_error("Discord closed the connection.");
        else if (opcode == 1) validate_rpc(nlohmann::json::parse(body));
        else throw std::runtime_error("Unexpected Discord IPC opcode.");
    }
}

void DiscordClient::clear() {
    if (!connected()) return;
    try {
        const nlohmann::json command{{"cmd", "SET_ACTIVITY"}, {"args", {{"pid", GetCurrentProcessId()}, {"activity", nullptr}}}, {"nonce", "shutdown"}};
        send_frame(1, command.dump(), GetTickCount64() + 500, false);
    } catch (const std::exception&) {
    }
    close();
}

}
