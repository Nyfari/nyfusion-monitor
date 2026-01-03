#pragma once
/**
 * @file MemoryProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include "../hardware/MemoryInfo.hpp"

namespace ny::domain::providers
{
    class MemoryProvider
    {
        public:
            virtual ~MemoryProvider() = default;
            virtual memory::MemoryInfo collectMemoryInfo() = 0;
    };
}