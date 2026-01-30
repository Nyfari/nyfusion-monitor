#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxCpuFrequencySensor.hpp
 * Created by Marcos Henrique
 * @date 29/01/2026
 *
 * @brief Classe que representa o sensor de frequência da CPU no Linux
 */

#ifndef NY_FUSION_MONITOR_LINUXCPUFREQUENCYSENSOR_HPP
#define NY_FUSION_MONITOR_LINUXCPUFREQUENCYSENSOR_HPP
#include <vector>
#include <optional>

#include "../../../common/cpu/ICpuFrequencySensor.hpp"
#include "common/ISensor.hpp"

namespace ny::infra::linux::sensor {

    class LinuxCpuFrequencySensor final
        : public ny::infra::common::ICpuFrequencySensor {

    public:
        LinuxCpuFrequencySensor();

        /// Atualiza leitura das frequências
        void update() override;

        /// Frequência por thread (MHz)
        std::vector<double> readPerThreadFrequencyMHz() const override;

        /// Frequência média da CPU (MHz)
        double readAverageFrequencyMHz() const override;

    private:
        std::vector<double> frequenciesMHz;
        std::optional<double> averageMHz;

        /// Lê a frequência de uma CPU específica (MHz)
        static double readFrequencyMHz(int cpuId);
    };

} // namespace ny::infra::linux::sensor

#endif // NY_FUSION_MONITOR_LINUXCPUFREQUENCYSENSOR_HPP