/**
 * @file LinuxProviderFactory.cpp
 * Created by
 * @author Marcos Henrique
 * @date 03/01/2026
 */
#include "LinuxProviderFactory.hpp"

#include "../GpuVendorDetector.hpp"
#include "../providers/LinuxCPUProvider.hpp"
#include "../providers/LinuxGPUProvider.hpp"
#include "../providers/LinuxMemoryProvider.hpp"
#include "../sensors/gpu/LinuxGenericGpuSensor.hpp"
#include "../sensors/gpu/LinuxRadeonGpuSensor.hpp"

#include <memory>

namespace ny::infra::linux {

    std::unique_ptr<ny::domain::providers::CPUProvider>
    LinuxProviderFactory::createCpuProvider() const {
        return std::make_unique<LinuxCPUProvider>();
    }

    std::unique_ptr<ny::domain::providers::MemoryProvider>
    LinuxProviderFactory::createMemoryProvider() const {
        return std::make_unique<LinuxMemoryProvider>();
    }

    std::unique_ptr<ny::domain::providers::GPUProvider>
    LinuxProviderFactory::createGpuProvider() const {
        const auto vendor = GpuVendorDetector::detectVendor();
        if (vendor == GpuVendor::AMD) {
            auto amdSensor = std::make_shared<sensor::LinuxRadeonGpuSensor>();
            return std::make_unique<LinuxGPUProvider>(
                std::static_pointer_cast<ny::infra::common::gpu::IGpuSensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuTemperatureSensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuUsageSensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuMemorySensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuFrequencySensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuPowerSensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuDriverSensor>(amdSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuFeatureSensor>(amdSensor)
            );
        }

        if (vendor == GpuVendor::NVIDIA || vendor == GpuVendor::Intel) {
            auto genericSensor = std::make_shared<sensor::LinuxGenericGpuSensor>(vendor);
            return std::make_unique<LinuxGPUProvider>(
                std::static_pointer_cast<ny::infra::common::gpu::IGpuSensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuTemperatureSensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuUsageSensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuMemorySensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuFrequencySensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuPowerSensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuDriverSensor>(genericSensor),
                std::static_pointer_cast<ny::infra::common::gpu::IGpuFeatureSensor>(genericSensor)
            );
        }

        return std::make_unique<LinuxGPUProvider>();
    }

} // namespace ny::infra::linux
