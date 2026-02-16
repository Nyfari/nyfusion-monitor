// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsCpuFrequencySensor.hpp
 * Source File Name: WindowsCpuFrequencySensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "WindowsCpuFrequencySensor.hpp"
#include "windows/readers/cpu/ProcessorPowerReader.hpp"
#include "windows/readers/cpu/LogicalProcessorReader.hpp"

namespace ny::infra::windows::sensor {

    void WindowsCpuFrequencySensor::update()
    {
        reader::ProcessorPowerReader powerReader;
        reader::LogicalProcessorReader topologyReader;

        const auto topoOpt = topologyReader.readCoreAndThreadCount();
        const auto sample = powerReader.read();

        m_perThreadFrequencies.clear();

        if (!topoOpt.has_value()) {
            m_averageFrequency = 0.0;
            return;
        }

        const auto [coreCount, threadCount] = topoOpt.value();
        const double freq = sample.currentMHz;

        if (freq <= 0.0) {
            m_averageFrequency = 0.0;
            return;
        }

        m_perThreadFrequencies.assign(threadCount, freq);
        m_averageFrequency = freq;
    }

    double WindowsCpuFrequencySensor::readAverageFrequencyMHz() const
    {
        return m_averageFrequency;
    }

    std::vector<double> WindowsCpuFrequencySensor::readPerThreadFrequencyMHz() const
    {
        return m_perThreadFrequencies;
    }

}
