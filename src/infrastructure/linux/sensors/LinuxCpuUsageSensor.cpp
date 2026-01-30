// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxCpuUsageSensor.hpp
 * Source File Name: LinuxCpuUsageSensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 */
#include "LinuxCpuUsageSensor.hpp"

#include <algorithm>

namespace ny::infra::linux::sensor {

    LinuxCpuUsageSensor::LinuxCpuUsageSensor()
        = default;

    void LinuxCpuUsageSensor::update() {
        prevTotal   = currTotal;
        prevPerCpu = currPerCpu;

        currTotal   = reader.readTotal();
        currPerCpu = reader.readPerCpu();

        hasPrevious = true;
    }

    double LinuxCpuUsageSensor::calculateUsage(const Sample& a,
                                               const Sample& b) {
        const long totalDelta = b.total - a.total;
        const long idleDelta  = b.idle  - a.idle;

        if (totalDelta <= 0)
            return 0.0;

        const double usage =
            100.0 * (1.0 - static_cast<double>(idleDelta) /
                               static_cast<double>(totalDelta));

        return std::clamp(usage, 0.0, 100.0);
    }

    double LinuxCpuUsageSensor::readTotalUsagePercent() const {
        if (!hasPrevious)
            return 0.0;

        return calculateUsage(prevTotal, currTotal);
    }

    std::vector<double> LinuxCpuUsageSensor::readPerThreadUsagePercent() const {
        std::vector<double> usage;

        if (!hasPrevious)
            return usage;

        const size_t count =
            std::min(prevPerCpu.size(), currPerCpu.size());

        usage.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            usage.push_back(
                calculateUsage(prevPerCpu[i], currPerCpu[i])
            );
        }

        return usage;
    }
}