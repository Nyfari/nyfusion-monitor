// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsMemoryProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_WINDOWSMEMORYPROVIDER_HPP
#define NY_FUSION_MONITOR_WINDOWSMEMORYPROVIDER_HPP

#pragma once
 /**
  * @file WindowsMemoryProvider.hpp
  */

#include "../domain/providers/MemoryProvider.hpp"
#include "common/IMemorySensor.hpp"
#include <memory>

namespace ny::infra::windows {

    class WindowsMemoryProvider final
        : public ny::domain::providers::MemoryProvider
    {
    public:
        WindowsMemoryProvider();
        explicit WindowsMemoryProvider(
            std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
        );

        ny::domain::hardware::MemoryInfo collect() const override;

    private:
        mutable std::unique_ptr<ny::infra::common::IMemorySensor> m_memorySensor;
    };

} // namespace ny::infra::windows

#endif //NY_FUSION_MONITOR_WINDOWSMEMORYPROVIDER_HPP
