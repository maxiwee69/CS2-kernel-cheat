#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#include <thread>
#include <filesystem>
#include <random>
#include "client.dll.hpp"
#include "offsets.hpp"
#include "sounds.h"
#include <SFML/audio.hpp>
#pragma comment(lib, "winmm.lib")


namespace fs = std::filesystem;

std::vector<sf::Int16>& getRandomSoundFile() {
    std::vector<std::vector<sf::Int16>*> sound_files = { &audioData1, &audioData2, &audioData3, &audioData4, &audioData5 };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, sound_files.size() - 1);

    return *sound_files[distr(gen)];
}

void playSound(const std::vector<sf::Int16>& soundData) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromSamples(&soundData[0], soundData.size(), 2, 44100)) {
        // Handle error
    }

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    // Wait for the sound to finish playing
    while (sound.getStatus() == sf::Sound::Playing) {
        sf::sleep(sf::milliseconds(100));
    }
}

static DWORD get_process_id(const wchar_t* process_name) {
    DWORD process_id = 0;

    HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snap_shot == INVALID_HANDLE_VALUE)
        return process_id;

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(decltype(entry));

    if (Process32FirstW(snap_shot, &entry) == TRUE) {
        // Check if the first handle is the one we want.
        if (_wcsicmp(process_name, entry.szExeFile) == 0)
            process_id = entry.th32ProcessID;
        else {
            while (Process32NextW(snap_shot, &entry) == TRUE) {
                if (_wcsicmp(process_name, entry.szExeFile) == 0) {
                    process_id = entry.th32ProcessID;
                    break;
                }
            }
        }
    }

    CloseHandle(snap_shot);

    return process_id;
}

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
    std::uintptr_t module_base = 0;

    // Snap-shot of process' modules (dlls).
    HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap_shot == INVALID_HANDLE_VALUE)
        return module_base;

    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof(decltype(entry));

    if (Module32FirstW(snap_shot, &entry) == TRUE) {
        if (wcsstr(module_name, entry.szModule) != nullptr)
            module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
        else {
            while (Module32NextW(snap_shot, &entry) == TRUE) {
                if (wcsstr(module_name, entry.szModule) != nullptr) {
                    module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                    break;
                }
            }
        }
    }

    CloseHandle(snap_shot);

    return module_base;
}

namespace driver {
    namespace codes {
        // Used to setup the driver.
        constexpr ULONG attach =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

        // Read process memory.
        constexpr ULONG read =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

        // Read process memory.
        constexpr ULONG write =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }  // namespace codes

    // Shared between user mode & kernel mode.
    struct Request {
        HANDLE process_id;

        PVOID target;
        PVOID buffer;

        SIZE_T size;
        SIZE_T return_size;
    };

    bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
        Request r;
        r.process_id = reinterpret_cast<HANDLE>(pid);

        return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr,
            nullptr);
    }

    template <class T>
    T read_memory(HANDLE driver_handle, const std::uintptr_t addr) {
        T temp = {};

        Request r;
        r.target = reinterpret_cast<PVOID>(addr);
        r.buffer = &temp;
        r.size = sizeof(T);

        DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

        return temp;
    }

    template <class T>
    void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value) {
        Request r;
        r.target = reinterpret_cast<PVOID>(addr);
        r.buffer = (PVOID)&value;
        r.size = sizeof(T);

        DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr,
            nullptr);
    }
}



static int counter = 0;

int main() {
    const DWORD pid = get_process_id(L"cs2.exe");

    if (pid == 0) {
        std::cout << "Failed to find cs2.\n";
        std::cin.get();
        return 1;
    }

    const HANDLE driver = CreateFileW(L"\\\\.\\lilyweeDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driver == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create our driver handle.\n";
        std::cin.get();
        return 1;
    }

    if (driver::attach_to_process(driver, pid) == true) {
        std::cout << "Attachment successful.\n";

        if (const std::uintptr_t client = get_module_base(pid, L"client.dll"); client != 0) {
            std::cout << "Client found.\n";

            int previousTotalHits = 0;

            while (true) {
                if (GetAsyncKeyState(VK_END))
                    break;

                const auto local_player_pawn = driver::read_memory<std::uintptr_t>(
                    driver, client + client_dll::dwLocalPlayerPawn);

                if (local_player_pawn == 0)
                    continue;

                const auto flags = driver::read_memory<std::uint32_t>(
                    driver, local_player_pawn + C_BaseEntity::m_fFlags);

                const bool in_air = flags & (1 << 0);
                const bool space_pressed = GetAsyncKeyState(VK_SPACE);
                const auto force_jump =
                    driver::read_memory<DWORD>(driver, client + client_dll::dwForceJump);
                /*

                if (space_pressed && in_air) {
                    Sleep(5);
                    driver::write_memory(driver, client + client_dll::dwForceJump, 65537);
                }
                else if (space_pressed && !in_air) {
                    driver::write_memory(driver, client + client_dll::dwForceJump, 256);
                }
                else if (!space_pressed && force_jump == 65537) {
                    driver::write_memory(driver, client + client_dll::dwForceJump, 256);
                }

                */
                uintptr_t pBulletServices = driver::read_memory<uintptr_t>(driver, static_cast<std::uintptr_t>(local_player_pawn) + 0x1718);
                int totalHits = driver::read_memory<int>(driver, static_cast<std::uintptr_t>(pBulletServices) + 0x40);

                if (totalHits != previousTotalHits) {
                    if (totalHits == 0 && previousTotalHits != 0) {
                        // `totalHits` changed from non-zero to zero, do not play hitsound
                    }
                    else {
                        // Play the hitsound
                        playSound(getRandomSoundFile());
                        std::cout << "Played sound\n";  // Print message to console
                    }

                    previousTotalHits = totalHits; // Update the previousTotalHits

                }

                const int triggerKey = VK_F2;

                if (GetAsyncKeyState(triggerKey)) {
                    uintptr_t playerAddress = driver::read_memory<uintptr_t>(driver, client + client_dll::dwLocalPlayerPawn); // Changed this line
                    uintptr_t entity_list = driver::read_memory<uintptr_t>(driver, client + client_dll::dwEntityList);
                    uintptr_t local_player = driver::read_memory<uintptr_t>(driver, client + client_dll::dwLocalPlayerPawn);

                    int local_team = driver::read_memory<int>(driver, local_player + C_BaseEntity::m_iTeamNum);
                    int crosshair_entity = driver::read_memory<int>(driver, local_player + C_CSPlayerPawnBase::m_iIDEntIndex);
                    int entityId = driver::read_memory<int>(driver, playerAddress + C_CSPlayerPawnBase::m_iIDEntIndex);

                    if (crosshair_entity != -1) {
                        uintptr_t entity_entry = driver::read_memory<uintptr_t>(driver, entity_list + 0x8 * (crosshair_entity >> 0x9) + 0x10);
                        uintptr_t entity = driver::read_memory<uintptr_t>(driver, entity_entry + 120 * (crosshair_entity & 0x1FF));

                        int team = driver::read_memory<int>(driver, entity + C_BaseEntity::m_iTeamNum);
                        int health = driver::read_memory<int>(driver, entity + C_BaseEntity::m_iHealth);
                        if (health > 0 && team != local_team) {
                            int random_duration1 = rand() % 30 + 10;
                            int random_duration2 = rand() % 40 + 10;

                            std::this_thread::sleep_for(std::chrono::milliseconds(random_duration));
                            std::this_thread::sleep_for(std::chrono::milliseconds(random_duration1));
                            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                            std::this_thread::sleep_for(std::chrono::milliseconds(random_duration2));
                            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

                            std::cout << "Waited " << random_duration + random_duration1 + random_duration2 << " milliseconds before firing\n";
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(00));
                }

            }
        }
    }

    CloseHandle(driver);

    std::cin.get();

    return 0;
}