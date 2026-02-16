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
 * @date 15/02/2026
 *
 * @brief
 */
#pragma once

#include <optional>
#include <cstdint>

namespace ny::infra::windows::reader {

    struct MemorySnapshot {
        uint64_t totalBytes;
        uint64_t freeBytes;
    };

    class WindowsMemoryReader {
    public:
        std::optional<MemorySnapshot> read() const;
    };

}
