/**
 * @file HardwareMonitorService.cpp
 * Created by
 * @author Marcos Henrique
 * @date 04/01/2026
 */
#include "HardwareMonitorService.hpp"

#include "providers/CPUProvider.hpp"
#include "providers/GPUProvider.hpp"
#include "providers/MemoryProvider.hpp"

namespace ny::application::services
{
    HardwareMonitorService::HardwareMonitorService(
        ny::domain::providers::CPUProvider& cpuProvider,
        ny::domain::providers::MemoryProvider& memoryProvider,
        ny::domain::providers::GPUProvider& gpuProvider
    )
        : m_cpuProvider(cpuProvider)
        , m_memoryProvider(memoryProvider)
        , m_gpuProvider(gpuProvider)
    {
    }

    ny::domain::hardware::CPUInfo HardwareMonitorService::collectCpu()
    {
        return m_cpuProvider.collect();
    }

    ny::domain::hardware::MemoryInfo HardwareMonitorService::collectMemory() const
    {
        return m_memoryProvider.collect();
    }

    ny::domain::hardware::GPUInfo HardwareMonitorService::collectGpu()
    {
        return m_gpuProvider.collect();
    }

    HardwareMonitorService::HardwareSnapshot HardwareMonitorService::collectSnapshot()
    {
        return HardwareSnapshot{collectCpu(), collectMemory(), collectGpu()};
    }
}
