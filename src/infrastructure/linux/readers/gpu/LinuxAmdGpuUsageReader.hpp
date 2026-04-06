// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuUsageReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <optional>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    class LinuxAmdGpuUsageReader final {
    public:
        [[nodiscard]] std::optional<float> read() const {
            const auto cardPath = sysfs::resolveAmdCardPath();
            if (!cardPath.has_value()) {
                return std::nullopt;
            }

            const auto gpuBusyPercent = sysfs::readNumber<float>(
                cardPath.value() / "device/gpu_busy_percent"
            );
            if (!gpuBusyPercent.has_value()) {
                return std::nullopt;
            }
            return gpuBusyPercent;
        }
    };

} // namespace ny::infra::linux::reader::gpu
