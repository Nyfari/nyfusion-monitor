// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
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

#pragma comment(lib, "PowrProf.lib")

namespace ny::infra::windows::reader {

    // Definição oficial documentada pela Microsoft
    typedef struct _PROCESSOR_POWER_INFORMATION {
        ULONG Number;
        ULONG MaxMhz;
        ULONG CurrentMhz;
        ULONG MhzLimit;
        ULONG MaxIdleState;
        ULONG CurrentIdleState;
    } PROCESSOR_POWER_INFORMATION;

    ProcessorPowerSample ProcessorPowerReader::read() const {
        ProcessorPowerSample sample{};

        SYSTEM_INFO sysInfo{};
        GetSystemInfo(&sysInfo);

        const ULONG cpuCount = sysInfo.dwNumberOfProcessors;

        if (cpuCount == 0) {
            return sample;
        }

        std::vector<PROCESSOR_POWER_INFORMATION> buffer(cpuCount);

        const auto status = CallNtPowerInformation(
            ProcessorInformation,
            nullptr,
            0,
            buffer.data(),
            static_cast<ULONG>(sizeof(PROCESSOR_POWER_INFORMATION) * cpuCount)
        );

        if (status != ERROR_SUCCESS) {
            return sample;
        }

        double totalMHz = 0.0;

        for (const auto& cpu : buffer) {
            totalMHz += static_cast<double>(cpu.CurrentMhz);
        }

        sample.currentMHz = totalMHz / cpuCount;

        return sample;
    }

}