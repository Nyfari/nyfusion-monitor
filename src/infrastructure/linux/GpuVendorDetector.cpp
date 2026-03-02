// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: GpuVendorDetector.hpp
 * Source File Name: GpuVendorDetector.cpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 */
#include "GpuVendorDetector.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace ny::infra::linux {

    static std::string readFile(const std::string& path) noexcept {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return "";
            }
            std::string content;
            std::getline(file, content);

            while (!content.empty() && std::isspace(content.back())) {
                content.pop_back();
            }
            return content;
        } catch (...) {
            return "";
        }
    }

    GpuVendor GpuVendorDetector::detectVendor() noexcept {
        std::string drmPath = "/sys/class/drm";

        if (!std::filesystem::exists(drmPath)) {
            return GpuVendor::Unknown;
        }

        try {
            for (const auto& entry : std::filesystem::directory_iterator(drmPath)) {
                if (!entry.is_directory()) continue;

                std::string name = entry.path().filename().string();
                if (!name.starts_with("card")) continue;

                std::string vendorFile = entry.path().string() + "/device/vendor";
                std::string vendor = readFile(vendorFile);

                if (vendor.empty()) continue;

                // AMD = 0x1002
                if (vendor.find("0x1002") != std::string::npos) {
                    return GpuVendor::AMD;
                }
                // NVIDIA = 0x10DE
                if (vendor.find("0x10DE") != std::string::npos) {
                    return GpuVendor::NVIDIA;
                }
                // Intel = 0x8086
                if (vendor.find("0x8086") != std::string::npos) {
                    return GpuVendor::Intel;
                }
            }
        } catch (...) {
            return GpuVendor::Unknown;
        }

        return GpuVendor::Unknown;
    }
}