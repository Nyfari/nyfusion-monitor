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
#include "windows/readers/cpu/ThermGuardCpuReader.hpp"

namespace ny::infra::windows::sensor {

    void WindowsCpuFrequencySensor::update()
    {
        reader::LogicalProcessorReader topologyReader;
        const auto topoOpt = topologyReader.readCoreAndThreadCount();

        if (!topoOpt.has_value()) {
            m_averageFrequency = 0.0;
            m_perThreadFrequencies.clear();
            return;
        }

        const auto [coreCount, threadCount] = topoOpt.value();

        // ============================================================
        // 1) PRIORIDADE: THERMGUARD
        // ============================================================

        reader::ThermGuardCpuReader tgReader;
        auto tgDataOpt = tgReader.readCpuInfo();

        if (tgDataOpt.has_value() &&
            tgDataOpt->clockMHz > 0)
        {
            double freq = static_cast<double>(tgDataOpt->clockMHz);

            m_perThreadFrequencies.assign(threadCount, freq);
            m_averageFrequency = freq;

            return;
        }

        // ============================================================
        // 2) FALLBACK: ProcessorPowerReader
        // ============================================================

        reader::ProcessorPowerReader powerReader;
        const auto sample = powerReader.read();

        if (sample.currentMHz <= 0.0) {
            m_averageFrequency = 0.0;
            m_perThreadFrequencies.clear();
            return;
        }

        m_perThreadFrequencies.assign(threadCount, sample.currentMHz);
        m_averageFrequency = sample.currentMHz;
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
