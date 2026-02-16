// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsMemorySensor.hpp
 * Source File Name: WindowsMemorySensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "WindowsMemorySensor.hpp"
#include "windows/readers/cpu/WindowsMemoryReader.hpp"

namespace ny::infra::windows::sensor {

    void WindowsMemorySensor::update()
    {
        reader::WindowsMemoryReader memoryReader;

        const auto snapshotOpt = memoryReader.read();

        if (!snapshotOpt.has_value()) {
            m_totalBytes = 0;
            m_usedBytes = 0;
            m_freeBytes = 0;
            m_usagePercent = 0.0;
            return;
        }

        const auto snapshot = snapshotOpt.value();

        m_totalBytes = snapshot.totalBytes;
        m_freeBytes = snapshot.freeBytes;
        m_usedBytes = m_totalBytes - m_freeBytes;

        if (m_totalBytes == 0) {
            m_usagePercent = 0.0;
        }
        else {
            m_usagePercent =
                (static_cast<double>(m_usedBytes) /
                    static_cast<double>(m_totalBytes)) * 100.0;
        }
    }

    std::uint64_t WindowsMemorySensor::readTotalMemory() const
    {
        return m_totalBytes;
    }

    std::uint64_t WindowsMemorySensor::readUsedMemory() const
    {
        return m_usedBytes;
    }

    std::uint64_t WindowsMemorySensor::readFreeMemory() const
    {
        return m_freeBytes;
    }

    double WindowsMemorySensor::readUsagePercent() const
    {
        return m_usagePercent;
    }

}
