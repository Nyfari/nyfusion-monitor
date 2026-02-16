// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsCpuUsageSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_WINDOWSCPUUSAGESENSOR_HPP
#define NY_FUSION_MONITOR_WINDOWSCPUUSAGESENSOR_HPP

#include <vector>
#include <cstdint>
#include "common/cpu/ICpuUsageSensor.hpp"

namespace ny::infra::windows::sensor {

    class WindowsCpuUsageSensor final : public ny::infra::common::ICpuUsageSensor {
    public:
        WindowsCpuUsageSensor() = default;
        ~WindowsCpuUsageSensor() override = default;

        void update() override;

        double readTotalUsagePercent() const override;
        std::vector<double> readPerThreadUsagePercent() const override;

    private:
        bool m_initialized{ false };
        std::uint64_t m_previousIdle{ 0 };
        std::uint64_t m_previousTotal{ 0 };

        double m_totalUsage{ 0.0 };
        std::vector<double> m_perThreadUsage;
    };

}


#endif //NY_FUSION_MONITOR_WINDOWSCPUUSAGESENSOR_HPP