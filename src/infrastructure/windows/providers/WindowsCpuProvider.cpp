// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsCPUProvider.hpp
 * Source File Name: WindowsCPUProvider.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
 /**
  * @file WindowsCPUProvider.cpp
  */

#include "WindowsCPUProvider.hpp"

#include <algorithm>

namespace ny::infra::windows {

    WindowsCPUProvider::WindowsCPUProvider()
        : WindowsCPUProvider(
            std::make_unique<ny::infra::windows::sensor::WindowsCpuInfoSensor>(),
            std::make_unique<ny::infra::windows::sensor::WindowsCpuFrequencySensor>(),
            std::make_unique<ny::infra::windows::sensor::WindowsCpuUsageSensor>(),
            std::make_unique<ny::infra::windows::sensor::WindowsCpuTemperatureSensor>()
        ) {
    }

    WindowsCPUProvider::WindowsCPUProvider(
        std::unique_ptr<ny::infra::windows::sensor::WindowsCpuInfoSensor> infoSensor,
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
    )
        : m_infoSensor(std::move(infoSensor))
        , m_frequencySensor(std::move(frequencySensor))
        , m_usageSensor(std::move(usageSensor))
        , m_temperatureSensor(std::move(temperatureSensor)) {
    }

    ny::domain::hardware::CPUInfo WindowsCPUProvider::collect()
    {
        using namespace ny::domain::hardware;

        CPUInfo info{};

        m_infoSensor->update();
        m_frequencySensor->update();
        m_usageSensor->update();
        m_temperatureSensor->update();

        info.name = m_infoSensor->readName();
        info.coreCount = m_infoSensor->readCoreCount();
        info.threadCount = m_infoSensor->readThreadCount();
        info.temperatureCelsius = m_temperatureSensor->readAverageTemperatureCelsius();
        info.averageFrequencyMHz = m_frequencySensor->readAverageFrequencyMHz();
        info.usagePercent = m_usageSensor->readTotalUsagePercent();
        info.powerWatts = std::nullopt;

        const auto frequencies = m_frequencySensor->readPerThreadFrequencyMHz();
        const auto usage = m_usageSensor->readPerThreadUsagePercent();

        // Evitar std::min com initializer_list para prevenir ambiguidade
        int threads = info.threadCount;
        threads = std::min(threads, static_cast<int>(frequencies.size()));
        threads = std::min(threads, static_cast<int>(usage.size()));
        if (threads < 0) threads = 0;

        info.threads.clear();
        info.threads.reserve(static_cast<size_t>(threads));

        for (int i = 0; i < threads; ++i) {
            // Construção explícita para máxima compatibilidade:
            // usar o tipo definido em `CPUInfo.hpp`: CPUThreadInfo tem campos: threadId, frequencyMHz, usagePercent
            CPUThreadInfo t;
            t.threadId = i;
            t.frequencyMHz = frequencies[i];
            t.usagePercent = usage[i];
            info.threads.push_back(t);
        }

        return info;
    }

} // namespace ny::infra::windows
