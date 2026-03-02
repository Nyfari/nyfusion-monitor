// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuUtilizationSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu
{
    /**
     * @brief Interface para leitura de utilização da GPU
     *
     * Responsável por expor percentual de carga (0.0 - 100.0).
     *
     * @note Nem todos os drivers expõem essa métrica em tempo real.
     */
    class IGpuUtilizationSensor : public virtual IGpuSensor
    {
    public:
        virtual ~IGpuUtilizationSensor() = default;

        /**
         * @brief Retorna percentual de utilização da GPU
         *
         * @return Utilização em % (0.0 - 100.0), ou std::nullopt se indisponível
         * @note Alguns drivers não monitoram utilização.
         */
        [[nodiscard]] virtual std::optional<float>
        readUtilizationPercent() const noexcept = 0;
    };
} // namespace infrastructure::gpu
