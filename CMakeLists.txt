cmake_minimum_required(VERSION 3.21)
project(SilentHill2DiscordPresence VERSION 0.1.6 LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This mod requires Windows x64.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(presence_core STATIC src/core.cpp)
target_include_directories(presence_core PUBLIC src third_party)
target_compile_definitions(presence_core PUBLIC UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0a00)

add_library(presence_windows STATIC src/windows.cpp)
target_link_libraries(presence_windows PUBLIC presence_core)

add_library(SH2DiscordPresence SHARED src/mod.cpp)
target_link_libraries(SH2DiscordPresence PRIVATE presence_windows)
set_target_properties(SH2DiscordPresence PROPERTIES PREFIX "" SUFFIX ".asi")

enable_testing()
add_executable(core_tests tests/core_tests.cpp)
target_link_libraries(core_tests PRIVATE presence_core)
add_test(NAME core COMMAND core_tests "${CMAKE_CURRENT_SOURCE_DIR}/config/SH2DiscordPresence.json")

add_executable(ipc_test_client tests/ipc_client.cpp)
target_link_libraries(ipc_test_client PRIVATE presence_windows)

add_library(steam_api_stub SHARED tests/steam_api_stub.cpp)
set_target_properties(steam_api_stub PROPERTIES PREFIX "")
add_executable(steam_reader_test tests/steam_reader.cpp)
target_link_libraries(steam_reader_test PRIVATE presence_windows)

add_executable(mod_test_host tests/mod_host.cpp)
target_compile_definitions(mod_test_host PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(mod_test_host PROPERTIES OUTPUT_NAME "SHProto-Win64-Shipping")

if(MSVC)
    foreach(target presence_core presence_windows SH2DiscordPresence core_tests ipc_test_client mod_test_host steam_api_stub steam_reader_test)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    endforeach()
endif()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(NAME artwork_files COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/artwork_files.py)
    add_test(NAME local_steam COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/local_steam_tests.py $<TARGET_FILE:steam_reader_test> $<TARGET_FILE:steam_api_stub>)
    add_test(NAME ipc COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/ipc_tests.py $<TARGET_FILE:ipc_test_client>)
    add_test(NAME mod_lifecycle COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/mod_lifecycle.py $<TARGET_FILE:mod_test_host> $<TARGET_FILE:SH2DiscordPresence>)
endif()
