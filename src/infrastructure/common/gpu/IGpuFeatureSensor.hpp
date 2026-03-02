// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuFeatureSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu {

    /**
     * @brief Interface para leitura de features suportadas pela GPU
     *
     * Responsável por expor capabilities como Ray Tracing, DLSS, etc.
     *
     * @note Esta interface é extensível para futuro (novos features sem quebra de contrato).
     */
    class IGpuFeatureSensor : public virtual IGpuSensor {
    public:
        virtual ~IGpuFeatureSensor() = default;

        /**
         * @brief Retorna lista de features suportadas
         *
         * @return Vetor de strings descrevendo features (ex: ["Ray Tracing", "RDNA 3"])
         * @note Pode estar vazio se não houver features detectadas.
         * @note Usa strings para permitir expansão sem quebra de contrato.
         */
        [[nodiscard]] virtual std::vector<std::string>
        readSupportedFeatures() const noexcept = 0;
    };

} // namespace infrastructure::gpu