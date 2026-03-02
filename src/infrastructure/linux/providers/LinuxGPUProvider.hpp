// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file LinuxGPUProvider.hpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 *
 * @brief Provider de GPU que detecta vendor e coleta informações
 */
#ifndef NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP
#define NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP

#include "../../../domain/providers/GPUProvider.hpp"
#include "common/ISensor.hpp"

#include <memory>

namespace ny::infra::linux {

    class LinuxGPUProvider final : public ny::domain::providers::GPUProvider {
    public:
        ny::domain::hardware::GPUInfo collect() override;

    private:
        // Inicializa sensor apropriado baseado no vendor
        std::unique_ptr<ny::infra::common::ISensor> initSensor() const;
    };

} // namespace ny::infra::linux

#endif //NY_FUSION_MONITOR_LINUXGPUPROVIDER_HPP