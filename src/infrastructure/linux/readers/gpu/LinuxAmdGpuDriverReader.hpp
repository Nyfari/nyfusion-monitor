// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuDriverReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <optional>
#include <string>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    class LinuxAmdGpuDriverReader final {
    public:
        [[nodiscard]] std::optional<std::string> read() const {
            const auto version = sysfs::readText("/sys/module/amdgpu/version");
            if (version.has_value() && !version->empty()) {
                return std::string("amdgpu ") + version.value();
            }
            return std::nullopt;
        }
    };

} // namespace ny::infra::linux::reader::gpu
