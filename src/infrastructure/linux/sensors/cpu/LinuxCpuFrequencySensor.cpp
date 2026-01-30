// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxCpuFrequencySensor.hpp
 * Source File Name: LinuxCpuFrequencySensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 */
#include "LinuxCpuFrequencySensor.hpp"
#include <fstream>
#include <filesystem>
#include <cctype>   // std::isdigit

namespace fs = std::filesystem;

namespace ny::infra::linux::sensor {

    LinuxCpuFrequencySensor::LinuxCpuFrequencySensor() {
        frequenciesMHz.clear();
        averageMHz = std::nullopt;
    }

    void LinuxCpuFrequencySensor::update() {
        frequenciesMHz.clear();
        averageMHz.reset();

        const fs::path cpuPath{"/sys/devices/system/cpu"};

        double sum = 0.0;
        int count = 0;

        for (const auto& entry : fs::directory_iterator(cpuPath)) {
            const auto name = entry.path().filename().string();

            if (!name.starts_with("cpu"))
                continue;

            // cpu0, cpu1, cpu2...
            const auto idStr = name.substr(3);
            if (idStr.empty() || !std::isdigit(idStr[0]))
                continue;

            const int cpuId = std::stoi(idStr);
            const double freq = readFrequencyMHz(cpuId);

            if (freq <= 0.0)
                continue;

            frequenciesMHz.push_back(freq);
            sum += freq;
            ++count;
        }

        if (count > 0)
            averageMHz = sum / count;
        else
            averageMHz = 0.0;
    }

    std::vector<double> LinuxCpuFrequencySensor::readPerThreadFrequencyMHz() const {
        return frequenciesMHz;
    }

    double LinuxCpuFrequencySensor::readAverageFrequencyMHz() const {
        return averageMHz.value_or(0.0);
    }

    double LinuxCpuFrequencySensor::readFrequencyMHz(int cpuId) {
        const fs::path path =
            fs::path("/sys/devices/system/cpu") /
            ("cpu" + std::to_string(cpuId)) /
            "cpufreq/scaling_cur_freq";

        std::ifstream file(path);
        if (!file.is_open())
            return 0.0;

        long freqKHz = 0;
        file >> freqKHz;

        return freqKHz > 0 ? freqKHz / 1000.0 : 0.0;
    }

} // namespace ny::infra::linux::sensor