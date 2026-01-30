// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: ProcStatReader.hpp
 * Source File Name: ProcStatReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 */
#include "ProcStatReader.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace ny::infra::linux::reader {

    static ProcStatSample parseLine(const std::string& line) {
        std::istringstream iss(line);
        std::string cpu;

        long user = 0, nice = 0, system = 0;
        long idle = 0, iowait = 0;
        long irq = 0, softirq = 0, steal = 0;

        iss >> cpu
            >> user >> nice >> system >> idle
            >> iowait >> irq >> softirq >> steal;

        ProcStatSample sample;
        sample.idle  = idle + iowait;
        sample.total = user + nice + system + idle +
                       iowait + irq + softirq + steal;

        return sample;
    }

    ProcStatSample ProcStatReader::readTotal() const {
        std::ifstream file("/proc/stat");
        std::string line;

        if (!file.is_open()) {
            return {};
        }

        while (std::getline(file, line)) {
            if (line.starts_with("cpu ")) {
                return parseLine(line);
            }
        }

        return {};
    }

    std::vector<ProcStatSample> ProcStatReader::readPerCpu() const {
        std::ifstream file("/proc/stat");
        std::string line;

        std::vector<ProcStatSample> result;

        if (!file.is_open()) {
            return result;
        }

        while (std::getline(file, line)) {
            // cpu0, cpu1, cpu2...
            if (line.starts_with("cpu") && !line.starts_with("cpu ")) {
                result.push_back(parseLine(line));
            }
        }

        return result;
    }
}
