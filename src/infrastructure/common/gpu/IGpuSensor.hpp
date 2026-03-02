// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file IGpuSensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 23/02/2026
 *
 * @brief Interface base para sensores de GPU
 *
 * Define o contrato obrigatório para qualquer sensor de GPU,
 * independentemente do vendor (AMD, NVIDIA, Intel) ou SO.
 *
 * Responsabilidades:
 * - update(): atualiza estado interno lendo do sistema
 * - read*(): são const e retornam apenas dados em cache
 *
 * @note Implementadores NÃO devem acessar SO diretamente.
 *       Usar Readers para abstrair I/O do sistema.
 */
#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <vector>

namespace ny::infra::common::gpu
{
    class IGpuSensor
    {
    public:
        virtual ~IGpuSensor() = default;

        /**
         * @brief Atualiza todas as métricas da GPU lendo do sistema
         *
         * Este método é o responsável por toda I/O do sistema.
         * Implementadores devem usar um Reader para acessar dados do SO.
         *
         * @note Deve ser noexcept. Falhas silenciosas retornam valores inválidos.
         */
        virtual void update() noexcept = 0;

        /**
         * @brief Retorna o vendor da GPU
         *
         * @return Vendor (ex: "AMD", "NVIDIA", "Intel")
         * @note Sempre disponível. Não deve ser vazio.
         */
        [[nodiscard]] virtual std::string readVendor() const noexcept = 0;

        /**
         * @brief Retorna o modelo/nome da GPU
         *
         * @return Modelo da GPU (ex: "RADEON RX 7900 XTX")
         * @note Sempre disponível. Não deve ser vazio.
         */
        [[nodiscard]] virtual std::string readModel() const noexcept = 0;

        /**
         * @brief Retorna VRAM total em bytes
         *
         * Este valor é imutável durante a execução.
         *
         * @return VRAM total em bytes (ex: 24000000000 para 24GB)
         * @note Sempre disponível. Nunca zero.
         */
        [[nodiscard]] virtual std::uint64_t readVramTotalBytes() const noexcept = 0;
    };
} // namespace infrastructure::gpu
