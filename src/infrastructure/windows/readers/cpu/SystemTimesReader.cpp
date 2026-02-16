// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: SystemTimesReader.hpp
 * Source File Name: SystemTimesReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "SystemTimesReader.hpp"

#include <windows.h>

namespace ny::infra::windows::reader {

    static std::uint64_t toUint64(const FILETIME& ft) {
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }

    SystemTimesSample SystemTimesReader::read() const {
        FILETIME idleTime{}, kernelTime{}, userTime{};

        if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
            return {};
        }

        const auto idle = toUint64(idleTime);
        const auto kernel = toUint64(kernelTime);
        const auto user = toUint64(userTime);

        SystemTimesSample sample;
        sample.idle = idle;
        sample.total = kernel + user;

        return sample;
    }

}