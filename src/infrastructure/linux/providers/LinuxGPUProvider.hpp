// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxGPUProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Provider de GPU que detecta vendor e coleta informações
 */
#ifndef NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP
#define NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP

#include "../../../domain/providers/GPUProvider.hpp"
#include "common/gpu/IGpuDriverSensor.hpp"
#include "common/gpu/IGpuFeatureSensor.hpp"
#include "common/gpu/IGpuFrequencySensor.hpp"
#include "common/gpu/IGpuMemorySensor.hpp"
#include "common/gpu/IGpuPowerSensor.hpp"
#include "common/gpu/IGpuSensor.hpp"
#include "common/gpu/IGpuTemperatureSensor.hpp"
#include "common/gpu/IGpuUsageSensor.hpp"

#include <memory>

namespace ny::infra::linux {

    class LinuxGPUProvider final : public ny::domain::providers::GPUProvider {
    public:
        LinuxGPUProvider() = default;
        LinuxGPUProvider(
            std::shared_ptr<ny::infra::common::gpu::IGpuSensor> sensor,
            std::shared_ptr<ny::infra::common::gpu::IGpuTemperatureSensor> temperatureSensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuUsageSensor> usageSensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuMemorySensor> memorySensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuFrequencySensor> frequencySensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuPowerSensor> powerSensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuDriverSensor> driverSensor = {},
            std::shared_ptr<ny::infra::common::gpu::IGpuFeatureSensor> featureSensor = {}
        );

        ny::domain::hardware::GPUInfo collect() override;

    private:
        std::shared_ptr<ny::infra::common::gpu::IGpuSensor> m_sensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuTemperatureSensor> m_temperatureSensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuUsageSensor> m_usageSensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuMemorySensor> m_memorySensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuFrequencySensor> m_frequencySensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuPowerSensor> m_powerSensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuDriverSensor> m_driverSensor;
        std::shared_ptr<ny::infra::common::gpu::IGpuFeatureSensor> m_featureSensor;
    };

} // namespace ny::infra::linux

#endif //NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP
