// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuTemperatureSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu {

    /**
     * @brief Interface para leitura de temperatura da GPU
     *
     * Responsável por expor temperatura em graus Celsius.
     *
     * @note Nem toda GPU possui sensor térmico acessível.
     */
    class IGpuTemperatureSensor : public virtual IGpuSensor {
    public:
        virtual ~IGpuTemperatureSensor() = default;

        /**
         * @brief Retorna temperatura da GPU em Celsius
         *
         * @return Temperatura em °C, ou std::nullopt se indisponível
         * @note Integradas/antigas podem não ter sensores térmicos.
         */
        [[nodiscard]] virtual std::optional<float>
        readTemperatureCelsius() const noexcept = 0;
    };

} // namespace infrastructure::gpu