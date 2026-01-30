// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ICpuUsageSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#include <vector>
#include "../ISensor.hpp"

namespace ny::infra::common {

    class ICpuUsageSensor : public ISensor {
    public:
        virtual ~ICpuUsageSensor() = default;

        virtual double readTotalUsagePercent() const = 0;
        virtual std::vector<double> readPerThreadUsagePercent() const = 0;
    };
}