// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuTemperatureReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <optional>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    class LinuxAmdGpuTemperatureReader final {
    public:
        [[nodiscard]] std::optional<float> read() const {
            const auto cardPath = sysfs::resolveAmdCardPath();
            if (!cardPath.has_value()) {
                return std::nullopt;
            }

            const auto hwmonPath = sysfs::resolveAmdHwmonPath(cardPath.value());
            if (!hwmonPath.has_value()) {
                return std::nullopt;
            }

            const auto milliCelsius = sysfs::readNumber<float>(hwmonPath.value() / "temp1_input");
            if (!milliCelsius.has_value()) {
                return std::nullopt;
            }
            return milliCelsius.value();
        }
    };

} // namespace ny::infra::linux::reader::gpu
