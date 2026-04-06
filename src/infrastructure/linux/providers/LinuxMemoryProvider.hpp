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
#include "common/IMemorySensor.hpp"
#include <memory>

namespace ny::infra::linux
{
    class LinuxMemoryProvider final
        : public ny::domain::providers::MemoryProvider
    {
    public:
        LinuxMemoryProvider();
        explicit LinuxMemoryProvider(
            std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
        );

        ny::domain::hardware::MemoryInfo collect() const override;

    private:
        mutable std::unique_ptr<ny::infra::common::IMemorySensor> m_memorySensor;
    };
}
#endif //NY_FUSION_MONITOR_LINUXMEMORYPROVIDER_HPP
