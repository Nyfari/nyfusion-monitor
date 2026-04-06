/**
 * @file MacMemoryProvider.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "MacMemoryProvider.hpp"

namespace ny::infra::mac {

    MacMemoryProvider::MacMemoryProvider(
        std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
    )
        : m_memorySensor(std::move(memorySensor)) {
    }

    ny::domain::hardware::MemoryInfo MacMemoryProvider::collect() const {
        m_memorySensor->update();

        ny::domain::hardware::MemoryInfo info(m_memorySensor->readTotalMemory());
        info.setUsedBytes(m_memorySensor->readUsedMemory());
        info.setFreeBytes(m_memorySensor->readFreeMemory());
        info.setUsagePercent(m_memorySensor->readUsagePercent());
        return info;
    }

} // namespace ny::infra::mac
