#include "windows.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <process.h>

namespace {

HMODULE module_handle = nullptr;
HANDLE stop_event = nullptr;
HANDLE worker_thread = nullptr;
volatile LONG initialized = 0;
volatile LONG shutting_down = 0;

struct InstanceGuard {
    HANDLE handle;
    ~InstanceGuard() { if (handle) CloseHandle(handle); }
};

std::filesystem::path module_path() {
    std::wstring buffer(32768, L'\0');
    const auto length = GetModuleFileNameW(module_handle, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (!length || length >= buffer.size()) throw std::runtime_error("Cannot locate the plugin.");
    buffer.resize(length);
    return std::filesystem::path(buffer);
}

void log_message(const std::filesystem::path& path, const std::string& message) noexcept {
    try {
        std::error_code error;
        if (std::filesystem::exists(path, error) && std::filesystem::file_size(path, error) > 1048576) {
            const auto previous = path.wstring() + L".previous";
            MoveFileExW(path.c_str(), previous.c_str(), MOVEFILE_REPLACE_EXISTING);
        }
        SYSTEMTIME now{};
        GetLocalTime(&now);
        std::ofstream file(path, std::ios::app | std::ios::binary);
        file << std::setfill('0') << now.wYear << '-' << std::setw(2) << now.wMonth << '-' << std::setw(2) << now.wDay
             << ' ' << std::setw(2) << now.wHour << ':' << std::setw(2) << now.wMinute << ':' << std::setw(2) << now.wSecond
             << " " << message << '\n';
    } catch (...) {
    }
}

void run_presence(HANDLE stop) {
    auto config_path = module_path();
    config_path.replace_extension(L".json");
    auto log_path = module_path();
    log_path.replace_extension(L".log");
    log_message(log_path, "Native ASI plugin 0.1.5 started inside the game.");
    const auto started = sh2presence::process_start_time();
    sh2presence::DiscordClient discord(stop);
    nlohmann::json previous_activity;
    std::string current_application;
    int current_pipe = -1;
    std::string previous_local_diagnostic;
    std::string previous_language;
    ULONGLONG next_poll = 0;
    while (WaitForSingleObject(stop, 0) == WAIT_TIMEOUT) {
        const auto now = GetTickCount64();
        if (now >= next_poll) {
            next_poll = now + 2000;
            try {
                const auto config = sh2presence::load_config(config_path);
                next_poll = now + static_cast<ULONGLONG>(config.poll_interval_seconds) * 1000;
                if (!config.enabled) {
                    discord.clear();
                    previous_activity = nullptr;
                    WaitForSingleObject(stop, 250);
                    continue;
                }
                sh2presence::SteamStatus status;
                try {
                    const auto presence = sh2presence::fetch_local_steam_presence();
                    status = sh2presence::steam_status_from_presence(presence.values, presence.game_language);
                    if (status.game_language != previous_language) {
                        log_message(log_path, "Detected game language: " + status.game_language);
                        previous_language = status.game_language;
                    }
                    const auto description = status.details.empty() ? "Steam has no readable local SH2 status yet; using generic text." : "Using the game's local Steam presence.";
                    if (description != previous_local_diagnostic) {
                        log_message(log_path, description);
                        previous_local_diagnostic = description;
                    }
                } catch (const std::exception& error) {
                    if (previous_local_diagnostic != error.what()) {
                        log_message(log_path, std::string("Local Steam presence not ready: ") + error.what());
                        previous_local_diagnostic = error.what();
                    }
                }
                if (WaitForSingleObject(stop, 0) != WAIT_TIMEOUT) break;
                const auto activity = sh2presence::make_activity(config, status, started);
                if (config.application_id != current_application || config.discord_pipe != current_pipe) discord.clear();
                const bool new_connection = !discord.connected();
                if (new_connection) {
                    discord.connect(config.application_id, config.discord_pipe);
                    current_application = config.application_id;
                    current_pipe = config.discord_pipe;
                }
                if (new_connection || activity != previous_activity) {
                    discord.update(activity);
                    previous_activity = activity;
                    log_message(log_path, "Discord confirmed activity: " + activity.at("details").get<std::string>());
                }
            } catch (const std::exception& error) {
                discord.clear();
                previous_activity = nullptr;
                log_message(log_path, std::string("Presence update failed: ") + error.what());
            }
        }
        if (discord.connected()) {
            try {
                discord.pump();
            } catch (const std::exception& error) {
                discord.close();
                previous_activity = nullptr;
                log_message(log_path, std::string("Discord disconnected: ") + error.what());
            }
        }
        WaitForSingleObject(stop, 250);
    }
    discord.clear();
    log_message(log_path, "Presence stopped and IPC connection closed.");
}

unsigned __stdcall worker_entry(void*) {
    try {
        wchar_t executable[32768]{};
        const auto length = GetModuleFileNameW(nullptr, executable, 32768);
        if (!length || length >= 32768) return 0;
        const auto filename = std::filesystem::path(executable).filename().wstring();
        if (_wcsicmp(filename.c_str(), L"SHProto-Win64-Shipping.exe") != 0) return 0;
        const auto instance_name = L"Local\\SH2DiscordPresence_" + std::to_wstring(GetCurrentProcessId());
        InstanceGuard instance{CreateMutexW(nullptr, FALSE, instance_name.c_str())};
        const auto instance_error = GetLastError();
        if (!instance.handle || instance_error == ERROR_ALREADY_EXISTS) return 0;
        HMODULE reference = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCWSTR>(&worker_entry), &reference)) return 0;
        run_presence(stop_event);
    } catch (...) {
    }
    return 0;
}

}

extern "C" __declspec(dllexport) void InitializeASI() {
    if (InterlockedCompareExchange(&initialized, 1, 0) != 0) return;
    stop_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!stop_event) return;
    worker_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, worker_entry, nullptr, 0, nullptr));
    if (!worker_thread) {
        CloseHandle(stop_event);
        stop_event = nullptr;
    }
}

extern "C" __declspec(dllexport) void ShutdownASI() {
    if (InterlockedCompareExchange(&shutting_down, 1, 0) != 0) return;
    if (!stop_event || !worker_thread) return;
    SetEvent(stop_event);
    if (WaitForSingleObject(worker_thread, 20000) == WAIT_OBJECT_0) {
        CloseHandle(worker_thread);
        CloseHandle(stop_event);
        worker_thread = nullptr;
        stop_event = nullptr;
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) module_handle = instance;
    return TRUE;
}
