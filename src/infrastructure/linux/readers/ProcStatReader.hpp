// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ProcStatReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
#ifndef NY_FUSION_MONITOR_PROCSTATREADER_HPP
#define NY_FUSION_MONITOR_PROCSTATREADER_HPP
#include <vector>

namespace ny::infra::linux::reader {

    struct ProcStatSample {
        long idle  = 0;
        long total = 0;
    };

    class ProcStatReader {
    public:
        /// Lê /proc/stat e retorna uma amostra por CPU
        /// - índice 0 = cpu0
        /// - NÃO inclui a linha "cpu" global
        std::vector<ProcStatSample> readPerCpu() const;

        /// Lê o total agregado da CPU ("cpu")
        ProcStatSample readTotal() const;
    };
}


#endif //NY_FUSION_MONITOR_PROCSTATREADER_HPP