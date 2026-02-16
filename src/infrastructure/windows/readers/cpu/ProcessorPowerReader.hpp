// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ProcessorPowerReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP
#define NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP

namespace ny::infra::windows::reader {

    struct ProcessorPowerSample {
        double currentMHz = 0.0;
    };

    class ProcessorPowerReader {
    public:
        ProcessorPowerSample read() const;
    };

}

#endif //NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP