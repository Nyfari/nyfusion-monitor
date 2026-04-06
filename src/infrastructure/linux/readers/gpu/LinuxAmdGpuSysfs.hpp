// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuSysfs.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace ny::infra::linux::reader::gpu::sysfs {

    inline std::optional<std::string> readText(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }

        std::string content;
        std::getline(file, content);
        while (!content.empty() && std::isspace(static_cast<unsigned char>(content.back()))) {
            content.pop_back();
        }

        return content;
    }

    template<typename T>
    inline std::optional<T> readNumber(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }

        T value{};
        file >> value;
        if (file.fail()) {
            return std::nullopt;
        }
        return value;
    }

    inline std::optional<std::filesystem::path> resolveAmdCardPath() {
        const std::filesystem::path drmPath{"/sys/class/drm"};
        if (!std::filesystem::exists(drmPath)) {
            return std::nullopt;
        }

        for (const auto& entry : std::filesystem::directory_iterator(drmPath)) {
            if (!entry.is_directory()) {
                continue;
            }

            const auto name = entry.path().filename().string();
            if (!name.starts_with("card")) {
                continue;
            }

            const auto vendor = readText(entry.path() / "device/vendor");
            if (!vendor.has_value()) {
                continue;
            }

            if (vendor.value() == "0x1002" || vendor.value() == "0x1002\n") {
                return entry.path();
            }
        }

        return std::nullopt;
    }

    inline std::optional<std::filesystem::path> resolveAmdHwmonPath(
        const std::filesystem::path& cardPath
    ) {
        const auto hwmonRoot = cardPath / "device/hwmon";
        if (!std::filesystem::exists(hwmonRoot)) {
            return std::nullopt;
        }

        for (const auto& entry : std::filesystem::directory_iterator(hwmonRoot)) {
            if (!entry.is_directory()) {
                continue;
            }

            const auto name = readText(entry.path() / "name");
            if (!name.has_value()) {
                continue;
            }

            if (name.value().find("amdgpu") != std::string::npos) {
                return entry.path();
            }
        }

        return std::nullopt;
    }

} // namespace ny::infra::linux::reader::gpu::sysfs
