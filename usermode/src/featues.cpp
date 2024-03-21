#include <Windows.h>
#include <atomic>
#include <future>
#include <thread>
#include <iostream>
#include <glm/glm.hpp>
#include "audio.hpp"
#include "memory.hpp"
#include "client.dll.hpp"
#include "offsets.hpp"
#include "sounds.hpp"
#include <sodium.h>
#include <vector>

#pragma comment (lib, "User32.lib")

uint32_t f() {
    uint32_t random_duration;
    randombytes_buf(&random_duration, sizeof(random_duration));
    random_duration %= 101; 
    return random_duration;
}

void handle_triggerbot(HANDLE driver, std::uintptr_t client) {
    bool triggerEnabled = false;
    bool messagePrinted = false;
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;
        const int triggerKey = 0x71; // F2 key
        if (GetAsyncKeyState(triggerKey) & 1) {
            triggerEnabled = !triggerEnabled;
            messagePrinted = false;
        }
        if (triggerEnabled && !messagePrinted) {
            std::cout << "Triggerbot is ON" << std::endl;
            messagePrinted = true;
        }
        else if (!triggerEnabled && !messagePrinted) {
            std::cout << "Triggerbot is OFF" << std::endl;
            messagePrinted = true;
        }
        if (triggerEnabled) {
            uintptr_t playerAddress = driver::read_memory<uintptr_t>(driver, client + client_dll::dwLocalPlayerPawn);
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
                    uint32_t random_duration1 = f();
                    uint32_t random_duration2 = f();
                    std::this_thread::sleep_for(std::chrono::milliseconds(random_duration1));
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                }
            }
        }
    }
}

void handle_bhop(HANDLE driver, std::uintptr_t client) {
    bool bhopEnabled = false;
    bool messagePrinted = false;
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;
        const int bhopKey = 0x73; // F4 Key
        if (GetAsyncKeyState(bhopKey) & 1) {
            bhopEnabled = !bhopEnabled;
            messagePrinted = false; 
        }
        if (bhopEnabled && !messagePrinted) {
            std::cout << "Bhop is ON" << std::endl;
            messagePrinted = true; 
        }
        else if (!bhopEnabled && !messagePrinted) {
            std::cout << "Bhop is OFF" << std::endl;
            messagePrinted = true; 
        }
        if (bhopEnabled) {
            const auto local_player_pawn = driver::read_memory<std::uintptr_t>(driver, client + client_dll::dwLocalPlayerPawn);
            if (local_player_pawn == 0)
                continue;
            const auto flags = driver::read_memory<std::uint32_t>(driver, local_player_pawn + C_BaseEntity::m_fFlags);
            const bool in_air = flags & (1 << 0);
            const bool space_pressed = GetAsyncKeyState(VK_SPACE);
            const auto force_jump = driver::read_memory<DWORD>(driver, client + client_dll::dwForceJump);

            if (space_pressed && in_air) {
                Sleep(10);
                driver::write_memory(driver, client + client_dll::dwForceJump, 65537);
            }
            else if (space_pressed && !in_air) {
                driver::write_memory(driver, client + client_dll::dwForceJump, 256);
            }
            else if (!space_pressed && force_jump == 65537) {
                driver::write_memory(driver, client + client_dll::dwForceJump, 256);
            }
        }
    }
}


void handle_hitsound(HANDLE driver, std::uintptr_t client, std::atomic<int>& previousTotalHits) {
    bool hitsoundEnabled = false;
    bool messagePrinted = false;
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;
        const int hitsoundKey = VK_F6; // F6 key
        if (GetAsyncKeyState(hitsoundKey) & 1) {
            hitsoundEnabled = !hitsoundEnabled;
            messagePrinted = false;
        }
        if (hitsoundEnabled && !messagePrinted) {
            std::cout << "Hitsound is ON" << std::endl;
            messagePrinted = true; 
        }
        else if (!hitsoundEnabled && !messagePrinted) {
            std::cout << "Hitsound is OFF" << std::endl;
            messagePrinted = true; 
        }
        if (hitsoundEnabled) {
            const auto local_player_pawn = driver::read_memory<std::uintptr_t>(driver, client + client_dll::dwLocalPlayerPawn);
            if (local_player_pawn == 0)
                continue;
            uintptr_t pBulletServices = driver::read_memory<uintptr_t>(driver, static_cast<std::uintptr_t>(local_player_pawn) + 0x1718);
            int totalHits = driver::read_memory<int>(driver, static_cast<std::uintptr_t>(pBulletServices) + 0x40);

            if (totalHits != previousTotalHits) {
                if (totalHits == 0 && previousTotalHits != 0) {
                }
                else {
                    playSound(getRandomSoundFile());
                }

                previousTotalHits = totalHits;
            }
        }
    }
}

