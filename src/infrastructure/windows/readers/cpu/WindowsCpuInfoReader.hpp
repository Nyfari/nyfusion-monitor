// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file WindowsCpuInfoReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#ifndef NY_FUSION_MONITOR_WINDOWSCPUINFOREADER_HPP
#define NY_FUSION_MONITOR_WINDOWSCPUINFOREADER_HPP

#include <string>

#include "windows/readers/cpu/LogicalProcessorReader.hpp"
#include "windows/readers/cpu/RegistryCpuReader.hpp"

namespace ny::infra::windows::reader {

    struct WindowsCpuInfoRaw final {
        std::string name{};
        int coreCount{0};
        int threadCount{0};
    };

    class WindowsCpuInfoReader final {
    public:
        [[nodiscard]] WindowsCpuInfoRaw read() const;

    private:
        RegistryCpuReader m_registryReader;
        LogicalProcessorReader m_logicalProcessorReader;
    };

} // namespace ny::infra::windows::reader

#endif // NY_FUSION_MONITOR_WINDOWSCPUINFOREADER_HPP
