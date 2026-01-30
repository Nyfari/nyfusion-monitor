// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ICpuPowerSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_ICPUPOWERSENSOR_HPP
#define NY_FUSION_MONITOR_ICPUPOWERSENSOR_HPP
#include "../ISensor.hpp"

namespace ny::infra::common {

    enum class CpuGovernor {
        Unknown,
        Performance,
        Powersave,
        Ondemand,
        Conservative,
        Schedutil
    };

    class ICpuPowerSensor : public ISensor {
    public:
        virtual ~ICpuPowerSensor() = default;

        virtual CpuGovernor readGovernor() const = 0;
        virtual bool isTurboEnabled() const = 0;
    };
}
#endif //NY_FUSION_MONITOR_ICPUPOWERSENSOR_HPP