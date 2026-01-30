// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * @file ISensor.hpp
 * Created by
 * @author Marcos Henrique
 * @date 29/01/2026
 *
 * @brief
 */
namespace ny::infra::common {

    class ISensor {
    public:
        virtual ~ISensor() = default;

        /// Atualiza o estado interno do sensor, se necessário
        /// Pode ser no-op para sensores stateless
        virtual void update() = 0;
    };

}
