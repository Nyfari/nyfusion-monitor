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

#include "../GpuVendorDetector.hpp"
#include "../sensors/gpu/LinuxRadeonGpuSensor.hpp"

namespace ny::infra::linux {

    std::unique_ptr<ny::infra::common::ISensor> LinuxGPUProvider::initSensor() const {
        GpuVendor vendor = GpuVendorDetector::detectVendor();

        switch (vendor) {
            case GpuVendor::AMD:
                return std::make_unique<sensor::LinuxRadeonGpuSensor>();
            case GpuVendor::NVIDIA:
                // TODO: Implementar LinuxNvidiaGpuSensor
                return nullptr;
            case GpuVendor::Intel:
                // TODO: Implementar LinuxIntelGpuSensor
                return nullptr;
            case GpuVendor::Unknown:
            default:
                return nullptr;
        }
    }

    ny::domain::hardware::GPUInfo LinuxGPUProvider::collect() {
        using namespace ny::domain::hardware;

        GPUInfo info{};

        // Instancia sensor apropriado
        auto sensor = initSensor();
        if (!sensor) {
            return info;  // Retorna vazio se nenhuma GPU detectada
        }

        // Atualiza sensor (lê sysfs)
        sensor->update();

        // Cast seguro para o sensor específico de GPU
        auto* gpuSensor = dynamic_cast<sensor::LinuxRadeonGpuSensor*>(sensor.get());
        if (!gpuSensor) {
            return info;
        }

        // Identificação
        info.vendor = gpuSensor->getVendor();
        info.model = gpuSensor->getModel();

        // VRAM total
        uint64_t vramBytes = gpuSensor->getVramTotalBytes();
        info.vramTotalBytes = vramBytes;
        info.vramTotalMB = vramBytes / (1024 * 1024);
        info.vramTotalGB = vramBytes / (1024 * 1024 * 1024);

        // VRAM em uso (opcional)
        uint64_t vramUsedBytes = gpuSensor->getVramUsedBytes();
        if (vramUsedBytes > 0) {
            info.vramUsedMB = vramUsedBytes / (1024 * 1024);
            info.vramUsedGB = vramUsedBytes / (1024 * 1024 * 1024);
        }

        // Métricas opcionais
        float utilization = gpuSensor->getUtilizationPercent();
        if (utilization > 0.0f) {
            info.utilizationPercent = utilization;
        }

        float temperature = gpuSensor->getTemperatureCelsius();
        if (temperature > 0.0f) {
            info.temperatureCelsius = temperature;
        }

        uint32_t frequency = gpuSensor->getFrequencyMHz();
        if (frequency > 0) {
            info.frequencyMHz = frequency;
        }

        float power = gpuSensor->getPowerWatts();
        if (power > 0.0f) {
            info.powerWatts = power;
        }

        // Driver e features
        info.driverVersion = gpuSensor->getDriverVersion();
        info.supportedFeatures = gpuSensor->getSupportedFeatures();

        return info;
    }

} // namespace ny::infra::linux