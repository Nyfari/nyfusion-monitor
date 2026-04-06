// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LinuxGPUProvider.hpp
 * Source File Name: LinuxGPUProvider.cpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 */
#include "LinuxGPUProvider.hpp"

namespace ny::infra::linux {

    LinuxGPUProvider::LinuxGPUProvider(
        std::shared_ptr<ny::infra::common::gpu::IGpuSensor> sensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuTemperatureSensor> temperatureSensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuUsageSensor> usageSensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuMemorySensor> memorySensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuFrequencySensor> frequencySensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuPowerSensor> powerSensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuDriverSensor> driverSensor,
        std::shared_ptr<ny::infra::common::gpu::IGpuFeatureSensor> featureSensor
    )
        : m_sensor(std::move(sensor))
        , m_temperatureSensor(std::move(temperatureSensor))
        , m_usageSensor(std::move(usageSensor))
        , m_memorySensor(std::move(memorySensor))
        , m_frequencySensor(std::move(frequencySensor))
        , m_powerSensor(std::move(powerSensor))
        , m_driverSensor(std::move(driverSensor))
        , m_featureSensor(std::move(featureSensor)) {
    }

    ny::domain::hardware::GPUInfo LinuxGPUProvider::collect() {
        using namespace ny::domain::hardware;

        GPUInfo info{};
        if (!m_sensor) {
            return info;
        }

        m_sensor->update();
        info.vendor = m_sensor->readVendor();
        info.model = m_sensor->readModel();
        info.vramTotalBytes = m_sensor->readVramTotalBytes();
        info.vramTotalMB = m_sensor->readVramTotalMB();
        info.vramTotalGB = m_sensor->readVramTotalGB();

        if (m_memorySensor) {
            info.vramUsedMB = m_memorySensor->readVramUsedMB();
            info.vramUsedGB = m_memorySensor->readVramUsedGB();
        }

        if (m_usageSensor) {
            info.utilizationPercent = m_usageSensor->readUsagePercent();
        }

        if (m_temperatureSensor) {
            info.temperatureCelsius = m_temperatureSensor->readTemperatureCelsius();
        }

        if (m_frequencySensor) {
            info.frequencyMHz = m_frequencySensor->readFrequencyMHz();
        }

        if (m_powerSensor) {
            info.powerWatts = m_powerSensor->readPowerWatts();
        }

        if (m_driverSensor) {
            info.driverVersion = m_driverSensor->readDriverVersion();
        }

        if (m_featureSensor) {
            info.supportedFeatures = m_featureSensor->readSupportedFeatures();
        }

        return info;
    }

} // namespace ny::infra::linux
