#pragma once
/**
 * @file GPUInfo.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include <string>
#include <cstdint>

namespace ny::domain::gpu
{
    class GPUInfo final
    {
        public:
            GPUInfo(
                std::string vendor,
                std::string model,
                std::uint64_t vramBytes
                );

            [[nodiscard]] const std::string& vendor() const noexcept;
            [[nodiscard]] const std::string& model() const noexcept;
            [[nodiscard]] std::uint64_t vramBytes() const noexcept;

    private:
        const std::string m_vendor;
        const std::string m_model;
        const std::uint64_t m_vramBytes;
    };
}