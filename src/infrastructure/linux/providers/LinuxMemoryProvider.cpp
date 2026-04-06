/**
 * @file LinuxMemoryProvider.cpp
 * Created by
 * @author Marcos Henrique
 * @date 04/01/2026
 */
#include "LinuxMemoryProvider.hpp"
#include "linux/sensors/ram/LinuxMemorySensor.hpp"

namespace ny::infra::linux
{
    LinuxMemoryProvider::LinuxMemoryProvider()
        : LinuxMemoryProvider(std::make_unique<sensor::LinuxMemorySensor>())
    {
    }

    LinuxMemoryProvider::LinuxMemoryProvider(
        std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
    )
        : m_memorySensor(std::move(memorySensor))
    {
    }

    ny::domain::hardware::MemoryInfo LinuxMemoryProvider::collect() const {
        m_memorySensor->update();

        ny::domain::hardware::MemoryInfo info(m_memorySensor->readTotalMemory());
        info.setUsedBytes(m_memorySensor->readUsedMemory());
        info.setFreeBytes(m_memorySensor->readFreeMemory());
        info.setUsagePercent(m_memorySensor->readUsagePercent());
        return info;
    }
}
