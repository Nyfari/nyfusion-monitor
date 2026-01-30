// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IMemorySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_IMEMORYSENSOR_HPP
#define NY_FUSION_MONITOR_IMEMORYSENSOR_HPP
#include <cstdint>
#include "ISensor.hpp"

namespace ny::infra::common {

    class IMemorySensor : public ISensor {
    public:
        virtual ~IMemorySensor() = default;

        /// Total de memória RAM (em bytes)
        virtual std::uint64_t readTotalMemory() const = 0;

        /// Memória usada (em bytes)
        virtual std::uint64_t readUsedMemory() const = 0;

        /// Memória livre (em bytes)
        virtual std::uint64_t readFreeMemory() const = 0;

        /// Percentual de uso da memória (0.0 - 100.0)
        virtual double readUsagePercent() const = 0;
    };

}
#endif //NY_FUSION_MONITOR_IMEMORYSENSOR_HPP