// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxCpuTemperatureSensor.hpp
 * Source File Name: LinuxCpuTemperatureSensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 */
#include "LinuxCpuTemperatureSensor.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace ny::infra::linux::sensor {

    LinuxCpuTemperatureSensor::LinuxCpuTemperatureSensor() = default;

    void LinuxCpuTemperatureSensor::update() {
        readHwmon();
    }

    double
    LinuxCpuTemperatureSensor::readAverageTemperatureCelsius() const {
        return averageCelsius.value_or(0.0);
    }

    std::vector<double>
    LinuxCpuTemperatureSensor::readPerCoreTemperatureCelsius() const {
        return perCoreCelsius;
    }

    void LinuxCpuTemperatureSensor::readHwmon() {
        perCoreCelsius.clear();
        averageCelsius.reset();

        for (const auto& hwmon : fs::directory_iterator("/sys/class/hwmon")) {
            std::ifstream nameFile(hwmon.path() / "name");
            if (!nameFile.is_open())
                continue;

            std::string name;
            std::getline(nameFile, name);

            if (name != "coretemp" && name != "k10temp")
                continue;

            double sum = 0.0;
            int count = 0;

            for (const auto& entry : fs::directory_iterator(hwmon.path())) {
                const auto filename = entry.path().filename().string();

                if (filename.starts_with("temp") && filename.ends_with("_input")) {
                    std::ifstream tempFile(entry.path());
                    if (!tempFile.is_open())
                        continue;

                    double milli;
                    tempFile >> milli;

                    const double celsius = milli / 1000.0;

                    perCoreCelsius.push_back(celsius);
                    sum += celsius;
                    ++count;
                }
            }

            if (count > 0) {
                averageCelsius = sum / count;
                return;
            }
        }
    }
}