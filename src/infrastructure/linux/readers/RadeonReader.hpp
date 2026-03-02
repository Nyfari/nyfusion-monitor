// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file RadeonReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Reader para GPU AMD Radeon - lê apenas dados brutos do sysfs
 */
#ifndef NY_FUSION_MONITOR_RADONREADER_HPP
#define NY_FUSION_MONITOR_RADONREADER_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace ny::infra::linux::reader {

    struct RadeonSample {
        std::string vendor;
        std::string model;
        uint64_t vramBytes = 0;
        uint64_t vramUsedBytes = 0;
        float utilizationPercent = 0.0f;
        float temperatureCelsius = 0.0f;
        uint32_t frequencyMHz = 0;
        float powerWatts = 0.0f;
        std::string driverVersion;
        std::vector<std::string> supportedFeatures;
    };

    class RadeonReader {
    public:
        /// Lê dados brutos da GPU AMD do sysfs
        /// Retorna amostra com todos os dados disponíveis
        RadeonSample readGpu() const;

    private:
        static std::string getGpuSysPath();
        static std::string getHwmonPath(const std::string& gpuPath);

        static std::string readFile(const std::string& path) noexcept;
        static uint64_t readUint64(const std::string& path) noexcept;
        static float readFloat(const std::string& path) noexcept;
        static uint32_t readUint32(const std::string& path) noexcept;
    };
}

#endif //NY_FUSION_MONITOR_RADONREADER_HPP