// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuFrequencyReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <cctype>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    class LinuxAmdGpuFrequencyReader final {
    public:
        [[nodiscard]] std::optional<std::uint32_t> read() const {
            const auto cardPath = sysfs::resolveAmdCardPath();
            if (!cardPath.has_value()) {
                return std::nullopt;
            }

            const auto hwmonPath = sysfs::resolveAmdHwmonPath(cardPath.value());
            if (hwmonPath.has_value()) {
                const auto rawFreq = sysfs::readNumber<std::uint64_t>(
                    hwmonPath.value() / "freq1_input"
                );
                if (rawFreq.has_value()) {
                    const auto mhz = normalizeToMHz(*rawFreq);
                    if (mhz.has_value() && *mhz > 0U) {
                        return mhz;
                    }
                }
            }

            const auto ppDpmMhz = readActiveClockFromPpDpm(
                cardPath.value() / "device/pp_dpm_sclk"
            );
            if (ppDpmMhz.has_value() && *ppDpmMhz > 0U) {
                return ppDpmMhz;
            }

            return std::nullopt;
        }

    private:
        static std::optional<std::uint32_t> normalizeToMHz(const std::uint64_t rawValue) {
            if (rawValue == 0U) {
                return std::nullopt;
            }

            std::uint64_t mhz = rawValue;
            if (rawValue >= 1000000ULL) {
                mhz = rawValue / 1000000ULL; // Hz -> MHz
            } else if (rawValue >= 1000ULL) {
                mhz = rawValue / 1000ULL; // kHz -> MHz
            }

            if (mhz == 0U || mhz > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
                return std::nullopt;
            }

            return static_cast<std::uint32_t>(mhz);
        }

        static std::optional<std::uint32_t> readActiveClockFromPpDpm(const std::filesystem::path& path) {
            const auto text = sysfs::readText(path);
            if (!text.has_value()) {
                return std::nullopt;
            }

            std::istringstream stream(text.value());
            std::string line;
            while (std::getline(stream, line)) {
                if (line.find('*') == std::string::npos) {
                    continue;
                }

                std::string lineLower = line;
                std::transform(
                    lineLower.begin(),
                    lineLower.end(),
                    lineLower.begin(),
                    [](const unsigned char ch) {
                        return static_cast<char>(std::tolower(ch));
                    }
                );

                const auto mhzPos = lineLower.find("mhz");
                if (mhzPos == std::string::npos) {
                    continue;
                }

                std::size_t start = mhzPos;
                while (start > 0 && std::isdigit(static_cast<unsigned char>(line[start - 1]))) {
                    --start;
                }

                if (start == mhzPos) {
                    continue;
                }

                const auto mhzText = line.substr(start, mhzPos - start);
                try {
                    const auto parsed = static_cast<std::uint32_t>(std::stoul(mhzText));
                    if (parsed > 0U) {
                        return parsed;
                    }
                } catch (...) {
                    continue;
                }
            }

            return std::nullopt;
        }
    };

} // namespace ny::infra::linux::reader::gpu
