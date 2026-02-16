// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file WmiTemperatureReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once

#ifndef NY_FUSION_MONITOR_WMITEMPERATUREREADER_HPP
#define NY_FUSION_MONITOR_WMITEMPERATUREREADER_HPP

#include <optional>

namespace ny::infra::windows::reader {

    class WmiTemperatureReader {
    public:
        std::optional<double> readTemperatureCelsius() const;
    };

}


#endif //NY_FUSION_MONITOR_WMITEMPERATUREREADER_HPP