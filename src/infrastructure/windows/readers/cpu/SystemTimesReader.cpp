// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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

namespace ny::infra::windows::reader {

    uint64_t SystemTimesReader::toUint64(const FILETIME& ft) {
        return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) |
               static_cast<uint64_t>(ft.dwLowDateTime);
    }

    SystemTimesSample SystemTimesReader::read() const {
        FILETIME idle{}, kernel{}, user{};
        GetSystemTimes(&idle, &kernel, &user);

        return {
            .idle   = toUint64(idle),
            .kernel = toUint64(kernel),
            .user   = toUint64(user)
        };
    }
}
#include "SystemTimesReader.hpp"