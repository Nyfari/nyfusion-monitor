// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxMemorySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_LINUXMEMORYSENSOR_HPP
#define NY_FUSION_MONITOR_LINUXMEMORYSENSOR_HPP
#include <cstdint>

#include "common/IMemorySensor.hpp"

namespace ny::infra::linux::sensor {

    class LinuxMemorySensor final : public ny::infra::common::IMemorySensor {
    public:
        LinuxMemorySensor() = default;

        void update() override;

        std::uint64_t readTotalMemory() const override;
        std::uint64_t readUsedMemory() const override;
        std::uint64_t readFreeMemory() const override;
        double readUsagePercent() const override;

    private:
        std::uint64_t total = 0;
        std::uint64_t free = 0;
        std::uint64_t used = 0;
        double usagePercent = 0.0;

        void readMemInfo();
    };

}
#endif //NY_FUSION_MONITOR_LINUXMEMORYSENSOR_HPP