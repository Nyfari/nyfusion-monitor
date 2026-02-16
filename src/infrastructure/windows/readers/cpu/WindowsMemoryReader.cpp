// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WmiTemperatureReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 15/02/2026
 *
 * @brief
 */
#include "WindowsMemoryReader.hpp"

#include <windows.h>

namespace ny::infra::windows::reader {

    std::optional<MemorySnapshot>
        WindowsMemoryReader::read() const {

        MEMORYSTATUSEX status{};
        status.dwLength = sizeof(status);

        if (!GlobalMemoryStatusEx(&status)) {
            return std::nullopt;
        }

        MemorySnapshot snapshot;
        snapshot.totalBytes = status.ullTotalPhys;
        snapshot.freeBytes = status.ullAvailPhys;

        return snapshot;
    }

}