// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsCpuUsageSensor.hpp
 * Source File Name: WindowsCpuUsageSensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "WindowsCpuUsageSensor.hpp"
#include "windows/readers/cpu/SystemTimesReader.hpp"
#include "windows/readers/cpu/LogicalProcessorReader.hpp"

namespace ny::infra::windows::sensor {

    void WindowsCpuUsageSensor::update()
    {
        reader::SystemTimesReader timesReader;
        reader::LogicalProcessorReader topologyReader;

        const auto sample = timesReader.read();
        const auto topoOpt = topologyReader.readCoreAndThreadCount();

        if (!topoOpt.has_value()) {
            m_totalUsage = 0.0;
            m_perThreadUsage.clear();
            return;
        }

        const auto [coreCount, threadCount] = topoOpt.value();

        const std::uint64_t idle = sample.idle;
        const std::uint64_t total = sample.total;

        if (!m_initialized) {
            m_previousIdle = idle;
            m_previousTotal = total;
            m_totalUsage = 0.0;
            m_perThreadUsage.assign(threadCount, 0.0);
            m_initialized = true;
            return;
        }

        const std::uint64_t deltaIdle = idle - m_previousIdle;
        const std::uint64_t deltaTotal = total - m_previousTotal;

        if (deltaTotal == 0) {
            m_totalUsage = 0.0;
        }
        else {
            double usage =
                (static_cast<double>(deltaTotal - deltaIdle) /
                    static_cast<double>(deltaTotal)) * 100.0;

            if (usage < 0.0) usage = 0.0;
            if (usage > 100.0) usage = 100.0;

            m_totalUsage = usage;
        }

        m_previousIdle = idle;
        m_previousTotal = total;

        m_perThreadUsage.assign(threadCount, m_totalUsage);
    }

    double WindowsCpuUsageSensor::readTotalUsagePercent() const
    {
        return m_totalUsage;
    }

    std::vector<double> WindowsCpuUsageSensor::readPerThreadUsagePercent() const
    {
        return m_perThreadUsage;
    }

}
