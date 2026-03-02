// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxRadeonGpuSensor.hpp
 * Source File Name: LinuxRadeonGpuSensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 */
#include "LinuxRadeonGpuSensor.hpp"

#include <algorithm>
#include <cctype>

namespace ny::infra::linux::sensor {

    LinuxRadeonGpuSensor::LinuxRadeonGpuSensor()
        = default;

    void LinuxRadeonGpuSensor::update() {
        // Lê dados brutos do Reader
        currentSample = reader.readGpu();

        // Detecta features baseado no modelo
        currentSample.supportedFeatures = detectFeatures(currentSample.model);
    }

    std::vector<std::string> LinuxRadeonGpuSensor::detectFeatures(const std::string& model) {
        std::vector<std::string> features;

        std::string modelLower = model;
        std::transform(modelLower.begin(), modelLower.end(), modelLower.begin(), ::tolower);

        // RDNA 3 (RX 7000 series)
        if (modelLower.find("navi 31") != std::string::npos ||
            modelLower.find("navi 32") != std::string::npos ||
            modelLower.find("rx 7") != std::string::npos) {
            features.push_back("RDNA 3");
            features.push_back("Ray Tracing");
            features.push_back("AV1 Encoding");
        }
        // RDNA 2 (RX 6000 series)
        else if (modelLower.find("navi 21") != std::string::npos ||
                 modelLower.find("navi 22") != std::string::npos ||
                 modelLower.find("navi 23") != std::string::npos ||
                 modelLower.find("rx 6") != std::string::npos) {
            features.push_back("RDNA 2");
            features.push_back("Ray Tracing");
        }
        // RDNA (RX 5000 series)
        else if (modelLower.find("navi 10") != std::string::npos ||
                 modelLower.find("navi 14") != std::string::npos ||
                 modelLower.find("rx 5") != std::string::npos) {
            features.push_back("RDNA");
        }
        // VEGA
        else if (modelLower.find("vega") != std::string::npos) {
            features.push_back("VEGA");
        }

        // Features genéricas
        if (modelLower.find("xt") != std::string::npos) {
            features.push_back("High Performance");
        }

        if (!features.empty()) {
            features.push_back("Modern Architecture");
            features.push_back("DCFX");
        }

        return features;
    }

    std::string LinuxRadeonGpuSensor::getVendor() const {
        return currentSample.vendor;
    }

    std::string LinuxRadeonGpuSensor::getModel() const {
        return currentSample.model;
    }

    uint64_t LinuxRadeonGpuSensor::getVramTotalBytes() const {
        return currentSample.vramBytes;
    }

    uint64_t LinuxRadeonGpuSensor::getVramUsedBytes() const {
        return currentSample.vramUsedBytes;
    }

    float LinuxRadeonGpuSensor::getUtilizationPercent() const {
        return std::clamp(currentSample.utilizationPercent, 0.0f, 100.0f);
    }

    float LinuxRadeonGpuSensor::getTemperatureCelsius() const {
        return currentSample.temperatureCelsius;
    }

    uint32_t LinuxRadeonGpuSensor::getFrequencyMHz() const {
        return currentSample.frequencyMHz;
    }

    float LinuxRadeonGpuSensor::getPowerWatts() const {
        return std::max(0.0f, currentSample.powerWatts);
    }

    std::string LinuxRadeonGpuSensor::getDriverVersion() const {
        return currentSample.driverVersion.empty() ? "Unknown" : currentSample.driverVersion;
    }

    std::vector<std::string> LinuxRadeonGpuSensor::getSupportedFeatures() const {
        return currentSample.supportedFeatures;
    }
}