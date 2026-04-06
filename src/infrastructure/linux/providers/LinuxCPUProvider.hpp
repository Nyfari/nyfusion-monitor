#pragma once
/**
 * @file LinuxCPUProvider.hpp
 * @author Marcos Henrique
 * @date 04/01/2026
 *
 * @brief Fornece informações de CPU no Linux usando sensores específicos.
 */

#include "../domain/providers/CPUProvider.hpp"
#include "common/cpu/ICpuFrequencySensor.hpp"
#include "common/cpu/ICpuTemperatureSensor.hpp"
#include "common/cpu/ICpuUsageSensor.hpp"
#include "linux/sensors/cpu/LinuxCpuInfoSensor.hpp"
#include "linux/sensors/cpu/LinuxCpuFrequencySensor.hpp"
#include "linux/sensors/cpu/LinuxCpuUsageSensor.hpp"
#include "linux/sensors/cpu/LinuxCpuTemperatureSensor.hpp"

#include <memory>

namespace ny::infra::linux {

    class LinuxCPUProvider final : public ny::domain::providers::CPUProvider {
    public:
        LinuxCPUProvider();
        LinuxCPUProvider(
            std::unique_ptr<ny::infra::linux::sensor::LinuxCpuInfoSensor> infoSensor,
            std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
            std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
            std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
        );

        ny::domain::hardware::CPUInfo collect() override;

    private:
        std::unique_ptr<ny::infra::linux::sensor::LinuxCpuInfoSensor> m_infoSensor;
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> m_frequencySensor;
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> m_usageSensor;
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> m_temperatureSensor;
    };

} // namespace ny::infra::linux
