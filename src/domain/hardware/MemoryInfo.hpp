#pragma once
/**
 * @file MemoryInfo.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include <cstdint>

namespace ny::domain::hardware {

    class MemoryInfo final {
    public:
        explicit MemoryInfo(std::uint64_t totalBytes = 0)
            : m_totalBytes(totalBytes), m_usedBytes(0), m_freeBytes(0), m_usagePercent(0.0) {}

        [[nodiscard]] std::uint64_t totalBytes() const noexcept { return m_totalBytes; }
        [[nodiscard]] std::uint64_t usedBytes() const noexcept { return m_usedBytes; }
        [[nodiscard]] std::uint64_t freeBytes() const noexcept { return m_freeBytes; }
        [[nodiscard]] double usagePercent() const noexcept { return m_usagePercent; }

        void setUsedBytes(std::uint64_t used) { m_usedBytes = used; }
        void setFreeBytes(std::uint64_t free) { m_freeBytes = free; }
        void setUsagePercent(double percent) { m_usagePercent = percent; }

    private:
        std::uint64_t m_totalBytes;
        std::uint64_t m_usedBytes;
        std::uint64_t m_freeBytes;
        double m_usagePercent;
    };

}