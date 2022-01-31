#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <vector>

namespace win
{
    static std::uintptr_t find_ygo_module()
    {
        std::vector<HMODULE> modules;
        modules.resize(1024);

        DWORD needed;
        EnumProcessModules(GetCurrentProcess(), modules.data(), modules.size(), &needed);
        modules.resize(needed);

        for (const auto& mod : modules)
        {
            wchar_t name[MAX_PATH] = L"";
            if (!GetModuleBaseNameW(GetCurrentProcess(), mod, name, sizeof(name) / sizeof(wchar_t)))
                continue;

            if (std::wcscmp(name, L"GameAssembly.dll") != 0)
                continue;

            return reinterpret_cast<std::uintptr_t>(mod);
        }

        return -1;
    }
}