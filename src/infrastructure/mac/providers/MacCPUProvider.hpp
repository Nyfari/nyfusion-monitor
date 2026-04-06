#pragma once
/**
 * @file MacCPUProvider.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_MACCPUPROVIDER_HPP
#define NY_FUSION_MONITOR_MACCPUPROVIDER_HPP

#include <memory>

#include "providers/CPUProvider.hpp"
#include "common/cpu/ICpuFrequencySensor.hpp"
#include "common/cpu/ICpuTemperatureSensor.hpp"
#include "common/cpu/ICpuUsageSensor.hpp"
#include "mac/sensors/cpu/MacCpuInfoSensor.hpp"

namespace ny::infra::mac {

    class MacCPUProvider final : public ny::domain::providers::CPUProvider {
    public:
        MacCPUProvider(
            std::unique_ptr<ny::infra::mac::sensor::MacCpuInfoSensor> infoSensor,
            std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
            std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
            std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
        );

        ny::domain::hardware::CPUInfo collect() override;

    private:
        std::unique_ptr<ny::infra::mac::sensor::MacCpuInfoSensor> m_infoSensor;
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> m_frequencySensor;
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> m_usageSensor;
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> m_temperatureSensor;
    };

} // namespace ny::infra::mac

#endif // NY_FUSION_MONITOR_MACCPUPROVIDER_HPP
