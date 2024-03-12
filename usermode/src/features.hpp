#pragma once
#include <windows.h>
#include <atomic>
#include "sounds.hpp"

void handle_bhop(HANDLE driver, std::uintptr_t client);
void handle_hitsound(HANDLE driver, std::uintptr_t client, std::atomic<int>& previousTotalHits);
void handle_triggerbot(HANDLE driver, std::uintptr_t client);