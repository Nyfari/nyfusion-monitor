// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file GpuVendorDetector.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Detector de vendor de GPU no Linux
 */
#ifndef NY_FUSION_MONITOR_GPUVENDORDETECTOR_HPP
#define NY_FUSION_MONITOR_GPUVENDORDETECTOR_HPP

namespace ny::infra::linux {

    enum class GpuVendor {
        AMD,
        NVIDIA,
        Intel,
        Unknown
    };

    class GpuVendorDetector {
    public:
        /// Detecta o vendor da GPU no sistema
        static GpuVendor detectVendor() noexcept;
    };
}

#endif //NY_FUSION_MONITOR_GPUVENDORDETECTOR_HPP