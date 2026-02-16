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
#include <string>

#include "../domain/hardware/CPUInfo.hpp"
#include "../domain/providers/CPUProvider.hpp"

#include "windows/sensors/cpu/WindowsCpuFrequencySensor.hpp"
#include "windows/sensors/cpu/WindowsCpuUsageSensor.hpp"
#include "windows/sensors/cpu/WindowsCpuTemperatureSensor.hpp"
#include "windows/readers/cpu/RegistryCpuReader.hpp"

namespace ny::infra::windows {

    class WindowsCPUProvider final : public ny::domain::providers::CPUProvider {
    public:
        WindowsCPUProvider() = default;
        ~WindowsCPUProvider() override = default;

        ny::domain::hardware::CPUInfo collect() override;

    private:
        ny::infra::windows::sensor::WindowsCpuFrequencySensor m_frequencySensor;
        ny::infra::windows::sensor::WindowsCpuUsageSensor m_usageSensor;
        ny::infra::windows::sensor::WindowsCpuTemperatureSensor m_temperatureSensor;

        static std::string readCpuName();
        static int countCores();
        static int countThreads();
    };

} // namespace ny::infra::windows
