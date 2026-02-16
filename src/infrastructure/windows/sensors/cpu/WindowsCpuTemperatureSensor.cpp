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
#include "windows/readers/cpu/KernelCpuTempReader.hpp"
#include "windows/readers/cpu/WmiTemperatureReader.hpp" // seu WMI robusto
#include "windows/readers/cpu/LogicalProcessorReader.hpp" // para core count
#include <optional>

namespace ny::infra::windows::sensor {

    using namespace ny::infra::windows::reader;

    void WindowsCpuTemperatureSensor::update()
    {
        // Primeiro: tentar via driver (MSR) por pacote e por core
        // Obter número de cores físicos (ou threads) para preencher per-core
        LogicalProcessorReader topoReader;
        const auto topoOpt = topoReader.readCoreAndThreadCount();
        int coreCount = 0;
        if (topoOpt.has_value()) {
            coreCount = topoOpt->first;
        }

        m_perCoreTemperatures.clear();

        // Tentar pacote via driver
        const auto packageTempOpt = KernelCpuTempReader::readTemperatureCelsius(-1);
        if (packageTempOpt.has_value()) {
            // Se driver disponível, tentar por-core também
            if (coreCount > 0) {
                m_perCoreTemperatures.resize(coreCount, packageTempOpt.value());
                // tentar ler por-core individualmente (se driver suportar)
                for (int i = 0; i < coreCount; ++i) {
                    auto coreTempOpt = KernelCpuTempReader::readTemperatureCelsius(i);
                    if (coreTempOpt.has_value()) {
                        m_perCoreTemperatures[i] = coreTempOpt.value();
                    }
                }
            }
            else {
                // sem topo info, apenas usar pacote
                m_perCoreTemperatures.push_back(packageTempOpt.value());
            }
            m_averageTemperature = packageTempOpt.value();
            return;
        }

        // Fallback: WMI robusto (MSAcpi_ThermalZoneTemperature)
        WmiTemperatureReader wmiReader;
        const auto wmiTempOpt = wmiReader.readTemperatureCelsius();
        if (wmiTempOpt.has_value()) {
            double c = wmiTempOpt.value();
            if (coreCount > 0) {
                m_perCoreTemperatures.assign(coreCount, c);
            }
            else {
                m_perCoreTemperatures.assign(1, c);
            }
            m_averageTemperature = c;
            return;
        }

        // Nenhuma fonte disponível: manter 0.0 (ou use std::nullopt se preferir)
        m_perCoreTemperatures.clear();
        m_averageTemperature = 0.0;
    }

    double WindowsCpuTemperatureSensor::readAverageTemperatureCelsius() const
    {
        return m_averageTemperature;
    }

    std::vector<double> WindowsCpuTemperatureSensor::readPerCoreTemperatureCelsius() const
    {
        return m_perCoreTemperatures;
    }

} // namespace ny::infra::windows::sensor
