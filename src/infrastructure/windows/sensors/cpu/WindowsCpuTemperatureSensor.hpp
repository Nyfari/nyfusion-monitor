// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsCpuTemperatureSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_WINDOWSCPUTEMPERATURESENSOR_HPP
#define NY_FUSION_MONITOR_WINDOWSCPUTEMPERATURESENSOR_HPP

#include <vector>
#include "common/cpu/ICpuTemperatureSensor.hpp"

namespace ny::infra::windows::sensor {

    class WindowsCpuTemperatureSensor final : public ny::infra::common::ICpuTemperatureSensor {
    public:
        WindowsCpuTemperatureSensor() = default;
        ~WindowsCpuTemperatureSensor() override = default;

        void update() override;

        double readAverageTemperatureCelsius() const override;
        std::vector<double> readPerCoreTemperatureCelsius() const override;

    private:
        double m_averageTemperature{ 0.0 };
        std::vector<double> m_perCoreTemperatures;
    };

}

#endif //NY_FUSION_MONITOR_WINDOWSCPUTEMPERATURESENSOR_HPP