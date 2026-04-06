// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuMemoryReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <cstdint>
#include <optional>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    struct LinuxAmdGpuMemoryRaw final {
        std::uint64_t vramTotalBytes{0};
        std::optional<std::uint64_t> vramUsedBytes{std::nullopt};
    };

    class LinuxAmdGpuMemoryReader final {
    public:
        [[nodiscard]] std::optional<LinuxAmdGpuMemoryRaw> read() const {
            const auto cardPath = sysfs::resolveAmdCardPath();
            if (!cardPath.has_value()) {
                return std::nullopt;
            }

            const auto totalBytes = sysfs::readNumber<std::uint64_t>(
                cardPath.value() / "device/mem_info_vram_total"
            );
            if (!totalBytes.has_value()) {
                return std::nullopt;
            }

            LinuxAmdGpuMemoryRaw raw{};
            raw.vramTotalBytes = totalBytes.value();
            raw.vramUsedBytes = sysfs::readNumber<std::uint64_t>(
                cardPath.value() / "device/mem_info_vram_used"
            );
            return raw;
        }
    };

} // namespace ny::infra::linux::reader::gpu
