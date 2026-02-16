// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LogicalProcessorReader.hpp
 * Created by
 * @author Marcos Henrique
 * @date 15/02/2026
 *
 * @brief
 */
#pragma once

#include <optional>

namespace ny::infra::windows::reader {

    class KernelCpuTempReader {
    public:
        // -1 = package, >=0 core index
        // Retorna temperatura em Celsius ou std::nullopt se driver ausente / leitura inválida
        static std::optional<double> readTemperatureCelsius(int coreIndex = -1);
    };

} // namespace ny::infra::windows::reader
