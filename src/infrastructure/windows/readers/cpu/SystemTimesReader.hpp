// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file SystemTimesReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once
#ifndef NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP
#define NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP

#include <cstdint>

namespace ny::infra::windows::reader {

    struct SystemTimesSample {
        std::uint64_t idle = 0;
        std::uint64_t total = 0;
    };

    class SystemTimesReader {
    public:
        SystemTimesSample read() const;
    };

}

#endif //NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP