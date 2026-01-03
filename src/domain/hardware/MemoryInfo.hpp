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

namespace ny::domain::memory {

    class MemoryInfo final {
    public:
        explicit MemoryInfo(std::uint64_t totalBytes);

        [[nodiscard]] std::uint64_t totalBytes() const noexcept;

    private:
        const std::uint64_t m_totalBytes;
    };

}