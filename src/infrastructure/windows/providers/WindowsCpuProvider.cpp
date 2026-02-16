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

#include <thread>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstdint>
#include <limits>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ny::infra::windows {

    std::string WindowsCPUProvider::readCpuName()
    {
        ny::infra::windows::reader::RegistryCpuReader localReader;
        const auto nameOpt = localReader.readProcessorName();
        return nameOpt.value_or(std::string{});
    }

    int WindowsCPUProvider::countCores()
    {
        DWORD len = 0;
        // primeira chamada para obter o tamanho necessário (em bytes)
        BOOL ok = GetLogicalProcessorInformation(nullptr, &len);
        if (!ok && GetLastError() != ERROR_INSUFFICIENT_BUFFER) return 0;

        if (len == 0) return 0;

        std::vector<uint8_t> buffer(len);
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.data());

        if (!GetLogicalProcessorInformation(info, &len)) return 0;

        const DWORD entrySize = static_cast<DWORD>(sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
        if (entrySize == 0) return 0;
        const DWORD count = len / entrySize;

        int physicalCores = 0;
        for (DWORD i = 0; i < count; ++i) {
            if (info[i].Relationship == RelationProcessorCore) {
                physicalCores++;
            }
        }
        return physicalCores;
    }

    int WindowsCPUProvider::countThreads()
    {
        const unsigned int threads = std::thread::hardware_concurrency();
        return threads > 0 ? static_cast<int>(threads) : 0;
    }

    ny::domain::hardware::CPUInfo WindowsCPUProvider::collect()
    {
        using namespace ny::domain::hardware;

        CPUInfo info{};

        info.name = readCpuName();
        info.coreCount = countCores();
        info.threadCount = countThreads();

        m_frequencySensor.update();
        m_usageSensor.update();
        m_temperatureSensor.update();

        const auto frequencies = m_frequencySensor.readPerThreadFrequencyMHz();
        const auto usage = m_usageSensor.readPerThreadUsagePercent();

        info.temperatureCelsius = m_temperatureSensor.readAverageTemperatureCelsius();

        // Evitar std::min com initializer_list para prevenir ambiguidade
        int threads = info.threadCount;
        threads = std::min(threads, static_cast<int>(frequencies.size()));
        threads = std::min(threads, static_cast<int>(usage.size()));
        if (threads < 0) threads = 0;

        double freqSum = 0.0;
        double usageSum = 0.0;

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

            freqSum += frequencies[i];
            usageSum += usage[i];
        }

        info.averageFrequencyMHz = threads > 0 ? freqSum / threads : 0.0;
        info.usagePercent = threads > 0 ? usageSum / threads : 0.0;
        info.powerWatts = std::nullopt;

        return info;
    }

} // namespace ny::infra::windows
