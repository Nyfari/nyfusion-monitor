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

namespace ny::infra::windows {

    class WindowsMemoryProvider final
        : public ny::domain::providers::MemoryProvider
    {
    public:
        ny::domain::hardware::MemoryInfo collect() const override;
    };

} // namespace ny::infra::windows

#endif //NY_FUSION_MONITOR_WINDOWSMEMORYPROVIDER_HPP