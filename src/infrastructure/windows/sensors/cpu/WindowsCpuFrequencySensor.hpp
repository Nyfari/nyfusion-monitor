// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsCpuFrequencySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_WINDOWSCPUFREQUENCYSENSOR_HPP
#define NY_FUSION_MONITOR_WINDOWSCPUFREQUENCYSENSOR_HPP

#include <vector>
#include "common/cpu/ICpuFrequencySensor.hpp"

namespace ny::infra::windows::sensor {

    class WindowsCpuFrequencySensor final : public ny::infra::common::ICpuFrequencySensor {
    public:
        WindowsCpuFrequencySensor() = default;
        ~WindowsCpuFrequencySensor() override = default;

        void update() override;

        double readAverageFrequencyMHz() const override;
        std::vector<double> readPerThreadFrequencyMHz() const override;

    private:
        double m_averageFrequency{ 0.0 };
        std::vector<double> m_perThreadFrequencies;
    };

}

#endif //NY_FUSION_MONITOR_WINDOWSCPUFREQUENCYSENSOR_HPP