#pragma once
/**
 * @file GPUProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include "../hardware/GPUInfo.hpp"

namespace ny::domain::providers
{
    class GPUProvider
    {
        public:
            virtual ~GPUProvider() = default;
            virtual gpu::GPUInfo collectGPUInfo() = 0;
    };
}