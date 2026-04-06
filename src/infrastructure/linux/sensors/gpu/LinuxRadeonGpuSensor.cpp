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

    void LinuxRadeonGpuSensor::update() noexcept {
        const auto identity = m_identityReader.read();
        if (!identity.has_value()) {
            m_vendor.clear();
            m_model.clear();
            m_vramTotalBytes = 0;
            m_vramTotalMB = 0;
            m_vramTotalGB = 0;
            m_vramUsedBytes.reset();
            m_vramUsedMB.reset();
            m_vramUsedGB.reset();
            m_usagePercent.reset();
            m_temperatureCelsius.reset();
            m_frequencyMHz.reset();
            m_powerWatts.reset();
            m_driverVersion.clear();
            m_supportedFeatures.clear();
            return;
        }

        m_vendor = identity->vendor;
        m_model = identity->model;

        const auto memory = m_memoryReader.read();
        if (memory.has_value()) {
            m_vramTotalBytes = memory->vramTotalBytes;
            m_vramTotalMB = static_cast<std::uint32_t>(m_vramTotalBytes / (1024ULL * 1024ULL));
            m_vramTotalGB = static_cast<std::uint32_t>(m_vramTotalBytes / (1024ULL * 1024ULL * 1024ULL));
            m_vramUsedBytes = memory->vramUsedBytes;
            if (m_vramUsedBytes.has_value()) {
                m_vramUsedMB = m_vramUsedBytes.value() / (1024ULL * 1024ULL);
                m_vramUsedGB = m_vramUsedBytes.value() / (1024ULL * 1024ULL * 1024ULL);
            } else {
                m_vramUsedMB.reset();
                m_vramUsedGB.reset();
            }
        } else {
            m_vramTotalBytes = 0;
            m_vramTotalMB = 0;
            m_vramTotalGB = 0;
            m_vramUsedBytes.reset();
            m_vramUsedMB.reset();
            m_vramUsedGB.reset();
        }

        const auto usage = m_usageReader.read();
        if (usage.has_value()) {
            m_usagePercent = std::clamp(usage.value(), 0.0f, 100.0f);
        } else {
            m_usagePercent.reset();
        }

        const auto milliCelsius = m_temperatureReader.read();
        if (milliCelsius.has_value()) {
            m_temperatureCelsius = milliCelsius.value() / 1000.0f;
        } else {
            m_temperatureCelsius.reset();
        }

        m_frequencyMHz = m_frequencyReader.read();
        if (m_frequencyMHz.has_value() && m_frequencyMHz.value() == 0U) {
            m_frequencyMHz.reset();
        }

        const auto microWatts = m_powerReader.read();
        if (microWatts.has_value()) {
            m_powerWatts = std::max(0.0f, microWatts.value() / 1000000.0f);
        } else {
            m_powerWatts.reset();
        }

        const auto driver = m_driverReader.read();
        m_driverVersion = driver.value_or(std::string{});
        m_supportedFeatures = detectFeatures(m_model);
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

    std::string LinuxRadeonGpuSensor::readVendor() const noexcept {
        return m_vendor;
    }

    std::string LinuxRadeonGpuSensor::readModel() const noexcept {
        return m_model;
    }

    std::uint64_t LinuxRadeonGpuSensor::readVramTotalBytes() const noexcept {
        return m_vramTotalBytes;
    }

    std::uint32_t LinuxRadeonGpuSensor::readVramTotalMB() const noexcept {
        return m_vramTotalMB;
    }

    std::uint32_t LinuxRadeonGpuSensor::readVramTotalGB() const noexcept {
        return m_vramTotalGB;
    }

    std::optional<std::uint64_t> LinuxRadeonGpuSensor::readVramUsedBytes() const noexcept {
        return m_vramUsedBytes;
    }

    std::optional<std::uint64_t> LinuxRadeonGpuSensor::readVramUsedMB() const noexcept {
        return m_vramUsedMB;
    }

    std::optional<std::uint64_t> LinuxRadeonGpuSensor::readVramUsedGB() const noexcept {
        return m_vramUsedGB;
    }

    std::optional<float> LinuxRadeonGpuSensor::readUsagePercent() const noexcept {
        return m_usagePercent;
    }

    std::optional<float> LinuxRadeonGpuSensor::readTemperatureCelsius() const noexcept {
        return m_temperatureCelsius;
    }

    std::optional<std::uint32_t> LinuxRadeonGpuSensor::readFrequencyMHz() const noexcept {
        return m_frequencyMHz;
    }

    std::optional<float> LinuxRadeonGpuSensor::readPowerWatts() const noexcept {
        return m_powerWatts;
    }

    std::string LinuxRadeonGpuSensor::readDriverVersion() const noexcept {
        return m_driverVersion;
    }

    std::vector<std::string> LinuxRadeonGpuSensor::readSupportedFeatures() const noexcept {
        return m_supportedFeatures;
    }
}
