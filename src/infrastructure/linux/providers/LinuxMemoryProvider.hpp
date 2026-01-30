#pragma once
/**
 * @file LinuxMemoryProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 04/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_LINUXMEMORYPROVIDER_HPP
#define NY_FUSION_MONITOR_LINUXMEMORYPROVIDER_HPP
#include "../domain/providers/MemoryProvider.hpp"
#include "common/ISensor.hpp"
#include <vector>
#include <memory>

namespace ny::infra::linux
{
    class LinuxMemoryProvider final
        : public ny::domain::providers::MemoryProvider
    {
    public:
        ny::domain::hardware::MemoryInfo collect() const override;

    private:
        std::vector<std::unique_ptr<common::ISensor>> initSensors() const;
    };
}
#endif //NY_FUSION_MONITOR_LINUXMEMORYPROVIDER_HPP
