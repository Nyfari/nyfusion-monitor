#pragma once
/**
 * @file CPUProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include "../hardware/CPUInfo.hpp"

namespace ny::domain::providers {

    class CPUProvider {
    public:
        virtual ~CPUProvider() = default;
        virtual ny::domain::hardware::CPUInfo collect() = 0;
    };
}
