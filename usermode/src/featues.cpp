#include <Windows.h>
#include <atomic>
#include <future>
#include <thread>
#include <random>
#include <iostream>
#include "audio.hpp"
#include "memory.hpp"
#include "client.dll.hpp"
#include "offsets.hpp"
#include "sounds.hpp"


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distr(0, 200);
int random_duration = distr(gen);

void handle_bhop(HANDLE driver, std::uintptr_t client) {
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;
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
        } else if (space_pressed && !in_air) {
            driver::write_memory(driver, client + client_dll::dwForceJump, 256);
        } else if (!space_pressed && force_jump == 65537) {
            driver::write_memory(driver, client + client_dll::dwForceJump, 256);
        }
    }
}

void handle_hitsound(HANDLE driver, std::uintptr_t client, std::atomic<int>& previousTotalHits) {
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;

        const auto local_player_pawn = driver::read_memory<std::uintptr_t>(
            driver, client + client_dll::dwLocalPlayerPawn);

        if (local_player_pawn == 0)
            continue;
        uintptr_t pBulletServices = driver::read_memory<uintptr_t>(driver, static_cast<std::uintptr_t>(local_player_pawn) + 0x1718);
        int totalHits = driver::read_memory<int>(driver, static_cast<std::uintptr_t>(pBulletServices) + 0x40);

        if (totalHits != previousTotalHits) {
            if (totalHits == 0 && previousTotalHits != 0) {
                // `totalHits` changed from non-zero to zero, do not play hitsound
            }
            else {
                // Play the hitsound
                playSound(getRandomSoundFile());
            }

            previousTotalHits = totalHits; // Update the previousTotalHits
        }
    }
}

void handle_triggerbot(HANDLE driver, std::uintptr_t client) {
    while (true) {
        if (GetAsyncKeyState(VK_END))
            break;
        const int triggerKey = VK_SHIFT;
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
                }
            }
        }
    }
}