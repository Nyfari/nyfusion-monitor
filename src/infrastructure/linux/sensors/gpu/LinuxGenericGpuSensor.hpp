// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxGenericGpuSensor.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include <optional>
#include <string>
#include <vector>

#include "common/gpu/IGpuDriverSensor.hpp"
#include "common/gpu/IGpuFeatureSensor.hpp"
#include "common/gpu/IGpuFrequencySensor.hpp"
#include "common/gpu/IGpuMemorySensor.hpp"
#include "common/gpu/IGpuPowerSensor.hpp"
#include "common/gpu/IGpuSensor.hpp"
#include "common/gpu/IGpuTemperatureSensor.hpp"
#include "common/gpu/IGpuUsageSensor.hpp"
#include "linux/GpuVendorDetector.hpp"

namespace ny::infra::linux::sensor {

    class LinuxGenericGpuSensor final
        : public ny::infra::common::gpu::IGpuTemperatureSensor
        , public ny::infra::common::gpu::IGpuUsageSensor
        , public ny::infra::common::gpu::IGpuMemorySensor
        , public ny::infra::common::gpu::IGpuFrequencySensor
        , public ny::infra::common::gpu::IGpuPowerSensor
        , public ny::infra::common::gpu::IGpuDriverSensor
        , public ny::infra::common::gpu::IGpuFeatureSensor {
    public:
        explicit LinuxGenericGpuSensor(GpuVendor vendor)
            : m_vendor(vendor == GpuVendor::NVIDIA ? "NVIDIA"
                      : vendor == GpuVendor::Intel ? "Intel"
                      : "Unknown")
            , m_model(m_vendor + " GPU")
        {
        }

        void update() noexcept override {}

        [[nodiscard]] std::string readVendor() const noexcept override { return m_vendor; }
        [[nodiscard]] std::string readModel() const noexcept override { return m_model; }
        [[nodiscard]] std::uint64_t readVramTotalBytes() const noexcept override { return 0; }
        [[nodiscard]] std::uint32_t readVramTotalMB() const noexcept override { return 0; }
        [[nodiscard]] std::uint32_t readVramTotalGB() const noexcept override { return 0; }
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedBytes() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedMB() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedGB() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<float> readUsagePercent() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<float> readTemperatureCelsius() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<std::uint32_t> readFrequencyMHz() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::optional<float> readPowerWatts() const noexcept override { return std::nullopt; }
        [[nodiscard]] std::string readDriverVersion() const noexcept override { return {}; }
        [[nodiscard]] std::vector<std::string> readSupportedFeatures() const noexcept override { return {}; }

    private:
        std::string m_vendor;
        std::string m_model;
    };

} // namespace ny::infra::linux::sensor
