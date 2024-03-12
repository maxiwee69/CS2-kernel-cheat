#include <Windows.h>
#include <TlHelp32.h>
#include "memory.hpp"
#include "client.dll.hpp"
#include "offsets.hpp"

DWORD get_process_id(const wchar_t* process_name) {
    DWORD process_id = 0;

    HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snap_shot == INVALID_HANDLE_VALUE)
        return process_id;

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(decltype(entry));

    if (Process32FirstW(snap_shot, &entry) != TRUE) {
        CloseHandle(snap_shot);
        return process_id;
    }

    if (_wcsicmp(process_name, entry.szExeFile) == 0) {
        process_id = entry.th32ProcessID;
    } else {
        do {
            if (_wcsicmp(process_name, entry.szExeFile) == 0) {
                process_id = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap_shot, &entry) == TRUE);
    }

    CloseHandle(snap_shot);

    return process_id;
}

std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
    std::uintptr_t module_base = 0;

    // Snap-shot of process' modules (dlls).
    HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap_shot == INVALID_HANDLE_VALUE)
        return module_base;

    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof(decltype(entry));

    if (Module32FirstW(snap_shot, &entry) != TRUE) {
        CloseHandle(snap_shot);
        return module_base;
    }

    do {
        if (wcsstr(module_name, entry.szModule) != nullptr) {
            module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
            break;
        }
    } while (Module32NextW(snap_shot, &entry) == TRUE);

    CloseHandle(snap_shot);

    return module_base;
}

namespace driver {
    bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
        Request r;
        r.process_id = reinterpret_cast<HANDLE>(pid);

        return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr,
            nullptr);
    }
}