// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxMemorySensor.hpp
 * Source File Name: LinuxMemorySensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 */
#include "LinuxMemorySensor.hpp"
#include <fstream>
#include <string>
#include <sstream>

namespace ny::infra::linux::sensor {

    void LinuxMemorySensor::update() {
        readMemInfo();
    }

    void LinuxMemorySensor::readMemInfo() {
        std::ifstream file("/proc/meminfo");
        std::string line;
        uint64_t memTotal = 0, memFree = 0, buffers = 0, cached = 0;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key;
            uint64_t value;
            std::string unit;

            iss >> key >> value >> unit;

            if (key == "MemTotal:") memTotal = value;
            else if (key == "MemFree:") memFree = value;
            else if (key == "Buffers:") buffers = value;
            else if (key == "Cached:") cached = value;
        }

        total = memTotal * 1024;           // converter de KB para bytes
        free = memFree * 1024 + buffers * 1024 + cached * 1024;
        used = total - free;
        usagePercent = total > 0 ? (double)used / total * 100.0 : 0.0;
    }

    std::uint64_t LinuxMemorySensor::readTotalMemory() const { return total; }
    std::uint64_t LinuxMemorySensor::readUsedMemory() const { return used; }
    std::uint64_t LinuxMemorySensor::readFreeMemory() const { return free; }
    double LinuxMemorySensor::readUsagePercent() const { return usagePercent; }

}
