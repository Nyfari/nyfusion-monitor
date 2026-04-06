// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxRadeonGpuSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Sensor que processa dados brutos da GPU AMD
 */
#ifndef NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP
#define NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP

#include <cstdint>
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
#include "linux/readers/gpu/LinuxAmdGpuDriverReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuFrequencyReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuIdentityReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuMemoryReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuPowerReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuTemperatureReader.hpp"
#include "linux/readers/gpu/LinuxAmdGpuUsageReader.hpp"

namespace ny::infra::linux::sensor {

    class LinuxRadeonGpuSensor final
        : public ny::infra::common::gpu::IGpuTemperatureSensor
        , public ny::infra::common::gpu::IGpuUsageSensor
        , public ny::infra::common::gpu::IGpuMemorySensor
        , public ny::infra::common::gpu::IGpuFrequencySensor
        , public ny::infra::common::gpu::IGpuPowerSensor
        , public ny::infra::common::gpu::IGpuDriverSensor
        , public ny::infra::common::gpu::IGpuFeatureSensor {

    public:
        LinuxRadeonGpuSensor() = default;

        void update() noexcept override;

        [[nodiscard]] std::string readVendor() const noexcept override;
        [[nodiscard]] std::string readModel() const noexcept override;
        [[nodiscard]] std::uint64_t readVramTotalBytes() const noexcept override;
        [[nodiscard]] std::uint32_t readVramTotalMB() const noexcept override;
        [[nodiscard]] std::uint32_t readVramTotalGB() const noexcept override;
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedBytes() const noexcept override;
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedMB() const noexcept override;
        [[nodiscard]] std::optional<std::uint64_t> readVramUsedGB() const noexcept override;
        [[nodiscard]] std::optional<float> readUsagePercent() const noexcept override;
        [[nodiscard]] std::optional<float> readTemperatureCelsius() const noexcept override;
        [[nodiscard]] std::optional<std::uint32_t> readFrequencyMHz() const noexcept override;
        [[nodiscard]] std::optional<float> readPowerWatts() const noexcept override;
        [[nodiscard]] std::string readDriverVersion() const noexcept override;
        [[nodiscard]] std::vector<std::string> readSupportedFeatures() const noexcept override;

    private:
        ny::infra::linux::reader::gpu::LinuxAmdGpuIdentityReader m_identityReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuMemoryReader m_memoryReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuUsageReader m_usageReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuTemperatureReader m_temperatureReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuFrequencyReader m_frequencyReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuPowerReader m_powerReader;
        ny::infra::linux::reader::gpu::LinuxAmdGpuDriverReader m_driverReader;

        std::string m_vendor{};
        std::string m_model{};
        std::uint64_t m_vramTotalBytes{0};
        std::uint32_t m_vramTotalMB{0};
        std::uint32_t m_vramTotalGB{0};
        std::optional<std::uint64_t> m_vramUsedBytes{std::nullopt};
        std::optional<std::uint64_t> m_vramUsedMB{std::nullopt};
        std::optional<std::uint64_t> m_vramUsedGB{std::nullopt};
        std::optional<float> m_usagePercent{std::nullopt};
        std::optional<float> m_temperatureCelsius{std::nullopt};
        std::optional<std::uint32_t> m_frequencyMHz{std::nullopt};
        std::optional<float> m_powerWatts{std::nullopt};
        std::string m_driverVersion{};
        std::vector<std::string> m_supportedFeatures{};

        static std::vector<std::string> detectFeatures(const std::string& model);
    };
}

#endif //NY_FUSION_MONITOR_LINUXRADEONGPUSENSOR_HPP
