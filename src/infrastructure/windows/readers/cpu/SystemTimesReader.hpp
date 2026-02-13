// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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
#ifndef NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP
#define NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP

#include <windows.h>
#include <cstdint>

namespace ny::infra::windows::reader {

    struct SystemTimesSample {
        uint64_t idle;
        uint64_t kernel;
        uint64_t user;
    };

    class SystemTimesReader {
    public:
        SystemTimesSample read() const;

    private:
        static uint64_t toUint64(const FILETIME& ft);
    };
}

#endif //NY_FUSION_MONITOR_SYSTEMTIMESREADER_HPP