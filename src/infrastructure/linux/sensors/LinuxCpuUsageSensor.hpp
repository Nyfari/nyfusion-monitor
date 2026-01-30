// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxCpuUsageSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_LINUXCPUUSAGESENSOR_HPP
#define NY_FUSION_MONITOR_LINUXCPUUSAGESENSOR_HPP
#include <vector>

#include "common/ICpuUsageSensor.hpp"
#include "common/ISensor.hpp"
#include "linux/readers/ProcStatReader.hpp"

namespace ny::infra::linux::sensor {

    class LinuxCpuUsageSensor final
        : public ny::infra::common::ICpuUsageSensor{

    public:
        LinuxCpuUsageSensor();

        /// Atualiza o estado interno (leitura do /proc/stat)
        void update() override;

        /// Uso percentual por thread (0.0 – 100.0)
        std::vector<double> readPerThreadUsagePercent() const override;

        /// Uso médio total da CPU
        double readTotalUsagePercent() const override;

    private:
        using Sample = ny::infra::linux::reader::ProcStatSample;

        ny::infra::linux::reader::ProcStatReader reader;

        Sample prevTotal{};
        Sample currTotal{};

        std::vector<Sample> prevPerCpu;
        std::vector<Sample> currPerCpu;

        bool hasPrevious = false;

        static double calculateUsage(const Sample& a,
                                     const Sample& b);
    };
}
#endif //NY_FUSION_MONITOR_LINUXCPUUSAGESENSOR_HPP