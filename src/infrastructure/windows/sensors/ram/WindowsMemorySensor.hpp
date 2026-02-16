// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsMemorySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_WINDOWSMEMORYSENSOR_HPP
#define NY_FUSION_MONITOR_WINDOWSMEMORYSENSOR_HPP

#include <cstdint>
#include "common/IMemorySensor.hpp"

namespace ny::infra::windows::sensor {

    class WindowsMemorySensor final : public ny::infra::common::IMemorySensor {
    public:
        WindowsMemorySensor() = default;
        ~WindowsMemorySensor() override = default;

        void update() override;

        std::uint64_t readTotalMemory() const override;
        std::uint64_t readUsedMemory() const override;
        std::uint64_t readFreeMemory() const override;
        double readUsagePercent() const override;

    private:
        std::uint64_t m_totalBytes{ 0 };
        std::uint64_t m_usedBytes{ 0 };
        std::uint64_t m_freeBytes{ 0 };
        double m_usagePercent{ 0.0 };
    };

}


#endif //NY_FUSION_MONITOR_WINDOWSMEMORYSENSOR_HPP