// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file PdhCpuReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_PDHCPUREADER_HPP
#define NY_FUSION_MONITOR_PDHCPUREADER_HPP
#include <windows.h>
#include <pdh.h>
#include <vector>

namespace ny::infra::windows::reader {

    class PdhCpuReader {
    public:
        PdhCpuReader();
        ~PdhCpuReader();

        std::vector<double> readPerLogicalProcessorUsage();

    private:
        PDH_HQUERY query = nullptr;
        PDH_HCOUNTER counter = nullptr;
    };
}

#endif //NY_FUSION_MONITOR_PDHCPUREADER_HPP