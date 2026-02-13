// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: ProcessorPowerReader.hpp
 * Source File Name: ProcessorPowerReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "ProcessorPowerReader.hpp"
#include <windows.h>
#include <powrprof.h>
#include <vector>

namespace ny::infra::windows::reader {

    std::vector<double>
    ProcessorPowerReader::readCurrentFrequenciesMHz() const {

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        ULONG size = sizeof(PROCESSOR_POWER_INFORMATION) *
                     sysInfo.dwNumberOfProcessors;

        std::vector<PROCESSOR_POWER_INFORMATION> buffer(
            sysInfo.dwNumberOfProcessors
        );

        CallNtPowerInformation(
            ProcessorInformation,
            nullptr,
            0,
            buffer.data(),
            size
        );

        std::vector<double> result;
        result.reserve(buffer.size());

        for (const auto& p : buffer)
            result.push_back(static_cast<double>(p.CurrentMhz));

        return result;
    }
}