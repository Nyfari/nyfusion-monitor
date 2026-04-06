#pragma once
/**
 * @file MacCpuInfoSensor.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_MACCPUINFOSENSOR_HPP
#define NY_FUSION_MONITOR_MACCPUINFOSENSOR_HPP

#include <string>
#include <utility>

#include "common/ISensor.hpp"
#include "mac/readers/MacCpuInfoReader.hpp"

namespace ny::infra::mac::sensor {

    class MacCpuInfoSensor final : public ny::infra::common::ISensor {
    public:
        explicit MacCpuInfoSensor(ny::infra::mac::reader::MacCpuInfoReader reader = {});

        void update() override;

        [[nodiscard]] const std::string& readName() const;
        [[nodiscard]] int readCoreCount() const;
        [[nodiscard]] int readThreadCount() const;

    private:
        ny::infra::mac::reader::MacCpuInfoReader m_reader;
        std::string m_name{};
        int m_coreCount{0};
        int m_threadCount{0};
    };

} // namespace ny::infra::mac::sensor

#endif // NY_FUSION_MONITOR_MACCPUINFOSENSOR_HPP
