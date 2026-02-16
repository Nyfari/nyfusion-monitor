// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: RegistryCpuReader.hpp
 * Source File Name: RegistryCpuReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "RegistryCpuReader.hpp"

#include <windows.h>

namespace ny::infra::windows::reader {

    std::optional<std::string> RegistryCpuReader::readProcessorName() const {

        HKEY hKey = nullptr;

        const char* subkey =
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            subkey,
            0,
            KEY_READ,
            &hKey) != ERROR_SUCCESS) {
            return std::nullopt;
        }

        char buffer[256]{};
        DWORD bufferSize = sizeof(buffer);

        const char* valueName = "ProcessorNameString";

        LONG status = RegQueryValueExA(hKey,
            valueName,
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(buffer),
            &bufferSize);

        RegCloseKey(hKey);

        if (status != ERROR_SUCCESS || bufferSize == 0) {
            return std::nullopt;
        }

        return std::string(buffer);
    }

}
