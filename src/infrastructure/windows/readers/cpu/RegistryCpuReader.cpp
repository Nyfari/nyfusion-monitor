// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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

    std::string RegistryCpuReader::readCpuName() const {

        HKEY key;
        if (RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                0,
                KEY_READ,
                &key) != ERROR_SUCCESS)
            return "Unknown CPU";

        char buffer[256];
        DWORD size = sizeof(buffer);

        RegQueryValueExA(
            key,
            "ProcessorNameString",
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(buffer),
            &size
        );

        RegCloseKey(key);

        return std::string(buffer);
    }
}