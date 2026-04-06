// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file WindowsCpuInfoReader.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "WindowsCpuInfoReader.hpp"

namespace ny::infra::windows::reader {

    WindowsCpuInfoRaw WindowsCpuInfoReader::read() const {
        WindowsCpuInfoRaw raw{};

        const auto nameOpt = m_registryReader.readProcessorName();
        raw.name = nameOpt.value_or(std::string{});

        const auto topologyOpt = m_logicalProcessorReader.readCoreAndThreadCount();
        if (topologyOpt.has_value()) {
            raw.coreCount = topologyOpt->first;
            raw.threadCount = topologyOpt->second;
        }

        return raw;
    }

} // namespace ny::infra::windows::reader
