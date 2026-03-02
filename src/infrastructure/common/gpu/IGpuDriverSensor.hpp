// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuDriverSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu {

    /**
     * @brief Interface para leitura de informações do driver da GPU
     *
     * Responsável por expor versão e metadados do driver.
     */
    class IGpuDriverSensor : public virtual IGpuSensor {
    public:
        virtual ~IGpuDriverSensor() = default;

        /**
         * @brief Retorna versão do driver
         *
         * @return Versão formatada (ex: "22.40.1")
         * @note Sempre disponível. Não deve ser vazio.
         */
        [[nodiscard]] virtual std::string
        readDriverVersion() const noexcept = 0;
    };

} // namespace infrastructure::gpu