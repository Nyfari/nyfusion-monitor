#pragma once
/**
 * @file HardwareMonitorService.hpp
 * Created by
 * @author Marcos Henrique
 * @date 04/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_HARDWAREMONITORSERVICE_HPP
#define NY_FUSION_MONITOR_HARDWAREMONITORSERVICE_HPP

#include "hardware/CPUInfo.hpp"
#include "hardware/GPUInfo.hpp"
#include "hardware/MemoryInfo.hpp"
#include "providers/CPUProvider.hpp"
#include "providers/GPUProvider.hpp"
#include "providers/MemoryProvider.hpp"

namespace ny::application::services
{
    class HardwareMonitorService final
    {
    public:
        struct HardwareSnapshot
        {
            ny::domain::hardware::CPUInfo cpu;
            ny::domain::hardware::MemoryInfo memory;
            ny::domain::hardware::GPUInfo gpu;
        };

        HardwareMonitorService(ny::domain::providers::CPUProvider& cpuProvider,
                               ny::domain::providers::MemoryProvider& memoryProvider,
                               ny::domain::providers::GPUProvider& gpuProvider);

        [[nodiscard]] ny::domain::hardware::CPUInfo collectCpu();
        [[nodiscard]] ny::domain::hardware::MemoryInfo collectMemory() const;
        [[nodiscard]] ny::domain::hardware::GPUInfo collectGpu();
        [[nodiscard]] HardwareSnapshot collectSnapshot();

    private:
        ny::domain::providers::CPUProvider& m_cpuProvider;
        ny::domain::providers::MemoryProvider& m_memoryProvider;
        ny::domain::providers::GPUProvider& m_gpuProvider;
    };
}

#endif //NY_FUSION_MONITOR_HARDWAREMONITORSERVICE_HPP
