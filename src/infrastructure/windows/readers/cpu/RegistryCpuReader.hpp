// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file RegistryCpuReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 *
 * @brief
 */
#pragma once

#include <string>
#include <optional>

namespace ny::infra::windows::reader {

    class RegistryCpuReader {
    public:
        std::optional<std::string> readProcessorName() const;
    };

}
