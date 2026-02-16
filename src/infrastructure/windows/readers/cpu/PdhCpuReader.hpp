// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
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
#pragma once
#ifndef NY_FUSION_MONITOR_PDHCpureader_HPP
#define NY_FUSION_MONITOR_PDHCpureader_HPP

#include <pdh.h>

namespace ny::infra::windows::reader {

    class PdhCpuReader {
    public:
        PdhCpuReader();
        ~PdhCpuReader();

        double readTotalUsage() const;

    private:
        PDH_HQUERY   query_ = nullptr;
        PDH_HCOUNTER counter_ = nullptr;
    };

}

#endif //NY_FUSION_MONITOR_PDHCPUREADER_HPP