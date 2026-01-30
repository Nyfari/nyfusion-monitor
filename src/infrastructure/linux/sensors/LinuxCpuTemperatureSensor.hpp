// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxCpuTemperatureSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_LINUXCPUTEMPERATURESENSOR_HPP
#define NY_FUSION_MONITOR_LINUXCPUTEMPERATURESENSOR_HPP
#include <optional>
#include <vector>

#include "common/ICpuTemperatureSensor.hpp"
#include "common/ISensor.hpp"

namespace ny::infra::linux::sensor {

    class LinuxCpuTemperatureSensor final
        : public ny::infra::common::ICpuTemperatureSensor {

    public:
        LinuxCpuTemperatureSensor();

        /// Atualiza leitura dos sensores térmicos
        void update() override;

        /// Temperatura média da CPU (°C)
        double readAverageTemperatureCelsius() const override;

        /// Temperatura por core físico (°C)
        /// Pode estar vazio se não suportado
        std::vector<double> readPerCoreTemperatureCelsius() const override;

    private:
        std::optional<double> averageCelsius;
        std::vector<double> perCoreCelsius;

        void readHwmon();
    };
}

#endif //NY_FUSION_MONITOR_LINUXCPUTEMPERATURESENSOR_HPP