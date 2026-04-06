#pragma once
/**
 * @file MacMemoryProvider.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_MACMEMORYPROVIDER_HPP
#define NY_FUSION_MONITOR_MACMEMORYPROVIDER_HPP

#include <memory>

#include "providers/MemoryProvider.hpp"
#include "common/IMemorySensor.hpp"

namespace ny::infra::mac {

    class MacMemoryProvider final : public ny::domain::providers::MemoryProvider {
    public:
        explicit MacMemoryProvider(
            std::unique_ptr<ny::infra::common::IMemorySensor> memorySensor
        );

        ny::domain::hardware::MemoryInfo collect() const override;

    private:
        mutable std::unique_ptr<ny::infra::common::IMemorySensor> m_memorySensor;
    };

} // namespace ny::infra::mac

#endif // NY_FUSION_MONITOR_MACMEMORYPROVIDER_HPP
