// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WindowsCpuTemperatureSensor.hpp
 * Source File Name: WindowsCpuTemperatureSensor.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "WindowsCpuTemperatureSensor.hpp"
#include "../include/ny/log/Logger.hpp"
#include "windows/readers/cpu/KernelCpuTempReader.hpp"
#include "windows/readers/cpu/WmiTemperatureReader.hpp"
#include "windows/readers/cpu/LogicalProcessorReader.hpp"
#include "windows/readers/cpu/ThermGuardCpuReader.hpp"
#include <optional>

namespace ny::infra::windows::sensor {

    using ny::log::Logger;
    using namespace ny::infra::windows::reader;

    void WindowsCpuTemperatureSensor::update() {

        Logger::log(ny::log::Level::Debug,
            "WindowsCpuTemperatureSensor", "update() start");

        LogicalProcessorReader topoReader;
        auto topoOpt = topoReader.readCoreAndThreadCount();
        int coreCount = topoOpt.has_value() ? topoOpt->first : 0;

        // ============================================================
        // 1) PRIORIDADE: THERMGUARD DRIVER
        // ============================================================

        ThermGuardCpuReader tgReader;
        auto tgTempOpt = tgReader.readTemperatureCelsius(-1);

        if (tgTempOpt.has_value()) {

            double temp = tgTempOpt.value();

            Logger::log(ny::log::Level::Info,
                "WindowsCpuTemperatureSensor",
                "Temperature obtained from ThermGuard driver: "
                + std::to_string(temp));

            m_averageTemperature = temp;
            m_perCoreTemperatures.assign(coreCount > 0 ? coreCount : 1, temp);

            Logger::log(ny::log::Level::Debug,
                "WindowsCpuTemperatureSensor",
                "update() end (thermguard)");

            return;
        }

        Logger::log(ny::log::Level::Debug,
            "WindowsCpuTemperatureSensor",
            "ThermGuard unavailable, trying kernel driver");

        // ============================================================
        // 2) Kernel Reader
        // ============================================================

        auto packageTempOpt = KernelCpuTempReader::readTemperatureCelsius(-1);

        if (packageTempOpt.has_value()) {

            double pkg = packageTempOpt.value();

            Logger::log(ny::log::Level::Info,
                "WindowsCpuTemperatureSensor",
                "Kernel package temp = " + std::to_string(pkg));

            m_averageTemperature = pkg;
            m_perCoreTemperatures.assign(coreCount > 0 ? coreCount : 1, pkg);

            Logger::log(ny::log::Level::Debug,
                "WindowsCpuTemperatureSensor",
                "update() end (kernel)");

            return;
        }

        Logger::log(ny::log::Level::Debug,
            "WindowsCpuTemperatureSensor",
            "Kernel unavailable, trying WMI");

        // ============================================================
        // 3) WMI Fallback
        // ============================================================

        WmiTemperatureReader wmi;
        auto wmiTempOpt = wmi.readTemperatureCelsius();

        if (wmiTempOpt.has_value()) {

            double t = wmiTempOpt.value();

            Logger::log(ny::log::Level::Info,
                "WindowsCpuTemperatureSensor",
                "WMI temp = " + std::to_string(t));

            m_averageTemperature = t;
            m_perCoreTemperatures.assign(coreCount > 0 ? coreCount : 1, t);

            Logger::log(ny::log::Level::Debug,
                "WindowsCpuTemperatureSensor",
                "update() end (wmi)");

            return;
        }

        // ============================================================
        // 4) No source
        // ============================================================

        Logger::log(ny::log::Level::Warn,
            "WindowsCpuTemperatureSensor",
            "No temperature source available; setting 0.0");

        m_averageTemperature = 0.0;
        m_perCoreTemperatures.clear();

        Logger::log(ny::log::Level::Debug,
            "WindowsCpuTemperatureSensor",
            "update() end (none)");
    }


    double WindowsCpuTemperatureSensor::readAverageTemperatureCelsius() const {
        return m_averageTemperature;
    }

    std::vector<double> WindowsCpuTemperatureSensor::readPerCoreTemperatureCelsius() const {
        return m_perCoreTemperatures;
    }

} // namespace ny::infra::windows::sensor
