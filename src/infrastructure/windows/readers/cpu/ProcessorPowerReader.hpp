// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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
#ifndef NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP
#define NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP
#include <vector>

namespace ny::infra::windows::reader {

    class ProcessorPowerReader {
    public:
        std::vector<double> readCurrentFrequenciesMHz() const;
    };
}

#endif //NY_FUSION_MONITOR_PROCESSORPOWERREADER_HPP