// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: ThermGuardCpuReader.hpp
 * Source File Name: ThermGuardCpuReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 16/02/2026
 */
#pragma once

#include <optional>
#include <string>

namespace ny::infra::windows::reader {

    class ThermGuardCpuReader {
    public:
        ThermGuardCpuReader() = default;
        ~ThermGuardCpuReader() = default;

        struct ThermGuardData {
            std::string brandString;
            uint32_t    clockMHz;
            int32_t     temperatureC;
            uint32_t    powerMilliWatts;
        };

        std::optional<ThermGuardData> readCpuInfo();
        std::optional<double> readTemperatureCelsius(int coreIndex = 0);

    private:
        ThermGuardCpuReader(const ThermGuardCpuReader&) = delete;
        ThermGuardCpuReader& operator=(const ThermGuardCpuReader&) = delete;
    };

}
