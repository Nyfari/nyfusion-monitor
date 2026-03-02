// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuFrequencySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu {

    /**
     * @brief Interface para leitura de frequência da GPU
     *
     * Responsável por expor frequência de clock em MHz.
     *
     * @note Frequência pode não estar disponível em todos os drivers.
     */
    class IGpuFrequencySensor : public virtual IGpuSensor {
    public:
        virtual ~IGpuFrequencySensor() = default;

        /**
         * @brief Retorna frequência atual da GPU em MHz
         *
         * @return Frequência em MHz, ou std::nullopt se indisponível
         * @note Frequência pode variar dinamicamente (DVFS).
         */
        [[nodiscard]] virtual std::optional<std::uint32_t>
        readFrequencyMHz() const noexcept = 0;
    };

} // namespace infrastructure::gpu