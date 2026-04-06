// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file IGpuUsageSensor.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu
{
    class IGpuUsageSensor : public virtual IGpuSensor
    {
    public:
        virtual ~IGpuUsageSensor() = default;

        [[nodiscard]] virtual std::optional<float>
        readUsagePercent() const noexcept = 0;
    };
}
