// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxRadeonGpuSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Sensor que processa dados brutos da GPU AMD
 */
#ifndef NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP
#define NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "../../../common/ISensor.hpp"
#include "linux/readers/RadeonReader.hpp"

namespace ny::infra::linux::sensor {

    class LinuxRadeonGpuSensor final
        : public ny::infra::common::ISensor {

    public:
        LinuxRadeonGpuSensor();

        /// Atualiza o estado interno (leitura do sysfs via RadeonReader)
        void update() override;

        // Getters para dados processados
        std::string getVendor() const;
        std::string getModel() const;
        uint64_t getVramTotalBytes() const;
        uint64_t getVramUsedBytes() const;
        float getUtilizationPercent() const;
        float getTemperatureCelsius() const;
        uint32_t getFrequencyMHz() const;
        float getPowerWatts() const;
        std::string getDriverVersion() const;
        std::vector<std::string> getSupportedFeatures() const;

    private:
        ny::infra::linux::reader::RadeonReader reader;
        ny::infra::linux::reader::RadeonSample currentSample;

        static std::vector<std::string> detectFeatures(const std::string& model);
    };
}

#endif //NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP