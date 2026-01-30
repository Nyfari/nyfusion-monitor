// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ICpuFrequencySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#include <vector>
#include "ISensor.hpp"

namespace ny::infra::common {

    class ICpuFrequencySensor : public ISensor {
    public:
        virtual ~ICpuFrequencySensor() = default;

        virtual double readAverageFrequencyMHz() const = 0;
        virtual std::vector<double> readPerThreadFrequencyMHz() const = 0;
    };
}