#pragma once
/**
 * @file CPUInfo.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include <string>
#include <cstdint>

namespace ny::domain::cpu
{
    class CPUInfo final
    {
    public:
        CPUInfo(
            std::string modelName,
            std::uint32_t coreCount,
            std::uint32_t threadCount,
            std::uint64_t baseFrequencyHz
        );

        [[nodiscard]] const std::string& modelName() const noexcept;
        [[nodiscard]] std::uint32_t coreCount() const noexcept;
        [[nodiscard]] std::uint32_t threadCount() const noexcept;
        [[nodiscard]] std::uint64_t baseFrequencyHz() const noexcept;

    private:
        const std::string m_modelName;
        const std::uint32_t m_coreCount;
        const std::uint32_t m_threadCount;
        const std::uint64_t m_baseFrequencyHz;
    };
}
