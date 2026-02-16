// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsMemoryProvider.hpp
 * Source File Name: WindowsMemoryProvider.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
 /**
  * @file WindowsMemoryProvider.cpp
  */

#include "WindowsMemoryProvider.hpp"

#include "windows/sensors/ram/WindowsMemorySensor.hpp"
#include "common/IMemorySensor.hpp"

namespace ny::infra::windows {

    ny::domain::hardware::MemoryInfo
        WindowsMemoryProvider::collect() const {

        using namespace ny::infra::windows::sensor;
        using namespace ny::infra::common;
        using namespace ny::domain::hardware;

        WindowsMemorySensor memSensor;

        memSensor.update();

        MemoryInfo info;

        info = MemoryInfo(memSensor.readTotalMemory());
        info.setUsedBytes(memSensor.readUsedMemory());
        info.setFreeBytes(memSensor.readFreeMemory());
        info.setUsagePercent(memSensor.readUsagePercent());

        return info;
    }

} // namespace ny::infra::windows
