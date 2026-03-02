// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuPowerSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu {

    /**
     * @brief Interface para leitura de potência consumida pela GPU
     *
     * Responsável por expor consumo de energia em Watts.
     *
     * @note Nem todas as GPUs possuem sensores de potência.
     */
    class IGpuPowerSensor : public virtual IGpuSensor {
    public:
        virtual ~IGpuPowerSensor() = default;

        /**
         * @brief Retorna consumo de potência da GPU em Watts
         *
         * @return Potência em W, ou std::nullopt se indisponível
         * @note Sensores de potência são menos comuns que temperatura/frequência.
         */
        [[nodiscard]] virtual std::optional<float>
        readPowerWatts() const noexcept = 0;
    };

} // namespace infrastructure::gpu