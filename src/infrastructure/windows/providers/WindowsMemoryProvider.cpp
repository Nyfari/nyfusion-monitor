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

    WindowsMemoryProvider::WindowsMemoryProvider()
        : WindowsMemoryProvider(std::make_unique<sensor::WindowsMemorySensor>())
    {
    }

    WindowsMemoryProvider::WindowsMemoryProvider(
        std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
    )
        : m_memorySensor(std::move(memorySensor))
    {
    }

    ny::domain::hardware::MemoryInfo
        WindowsMemoryProvider::collect() const {

        m_memorySensor->update();

        ny::domain::hardware::MemoryInfo info(m_memorySensor->readTotalMemory());
        info.setUsedBytes(m_memorySensor->readUsedMemory());
        info.setFreeBytes(m_memorySensor->readFreeMemory());
        info.setUsagePercent(m_memorySensor->readUsagePercent());
        return info;
    }

} // namespace ny::infra::windows
