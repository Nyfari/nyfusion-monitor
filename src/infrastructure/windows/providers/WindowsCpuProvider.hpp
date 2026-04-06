// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WindowsCPUProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
 /**
  * @file WindowsCPUProvider.hpp
  */
  // windows/providers/WindowsCPUProvider.hpp

#pragma once

#include <memory>

#include "../domain/hardware/CPUInfo.hpp"
#include "../domain/providers/CPUProvider.hpp"

#include "common/cpu/ICpuFrequencySensor.hpp"
#include "common/cpu/ICpuTemperatureSensor.hpp"
#include "common/cpu/ICpuUsageSensor.hpp"
#include "windows/sensors/cpu/WindowsCpuInfoSensor.hpp"
#include "windows/sensors/cpu/WindowsCpuFrequencySensor.hpp"
#include "windows/sensors/cpu/WindowsCpuUsageSensor.hpp"
#include "windows/sensors/cpu/WindowsCpuTemperatureSensor.hpp"

namespace ny::infra::windows {

    class WindowsCPUProvider final : public ny::domain::providers::CPUProvider {
    public:
        WindowsCPUProvider();
        WindowsCPUProvider(
            std::unique_ptr<ny::infra::windows::sensor::WindowsCpuInfoSensor> infoSensor,
            std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
            std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
            std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
        );

        ~WindowsCPUProvider() override = default;

        ny::domain::hardware::CPUInfo collect() override;

    private:
        std::unique_ptr<ny::infra::windows::sensor::WindowsCpuInfoSensor> m_infoSensor;
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> m_frequencySensor;
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> m_usageSensor;
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> m_temperatureSensor;
    };

} // namespace ny::infra::windows
