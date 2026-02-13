// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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
#ifndef NY_FUSION_MONITOR_REGISTRYCPUREADER_HPP
#define NY_FUSION_MONITOR_REGISTRYCPUREADER_HPP
#include <string>

namespace ny::infra::windows::reader {

    class RegistryCpuReader {
    public:
        std::string readCpuName() const;
    };
}

#endif //NY_FUSION_MONITOR_REGISTRYCPUREADER_HPP