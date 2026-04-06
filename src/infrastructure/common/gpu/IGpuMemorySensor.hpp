// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuMemorySensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 */
#pragma once

#include "IGpuSensor.hpp"

namespace ny::infra::common::gpu
{
    /**
     * @brief Interface para leitura de métricas de memória VRAM
     *
     * Responsável por expor dados de VRAM em uso.
     *
     * @note VRAM usada pode não estar disponível em todos os drivers.
     *       Use std::optional para indicar indisponibilidade.
     */
    class IGpuMemorySensor : public virtual IGpuSensor
    {
    public:
        virtual ~IGpuMemorySensor() = default;

        /**
         * @brief Retorna VRAM atualmente em uso
         *
         * @return VRAM usada em bytes, ou std::nullopt se indisponível
         * @note Alguns drivers (ex: integradas) não expõem essa métrica.
         */
        [[nodiscard]] virtual std::optional<std::uint64_t>
        readVramUsedBytes() const noexcept = 0;

        /**
         * @brief Retorna VRAM usada em MB
         */
        [[nodiscard]] virtual std::optional<std::uint64_t>
        readVramUsedMB() const noexcept = 0;

        /**
         * @brief Retorna VRAM usada em GB
         */
        [[nodiscard]] virtual std::optional<std::uint64_t>
        readVramUsedGB() const noexcept = 0;
    };
} // namespace infrastructure::gpu
