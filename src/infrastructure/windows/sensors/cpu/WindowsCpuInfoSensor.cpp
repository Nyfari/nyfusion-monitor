// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file WindowsCpuInfoSensor.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "WindowsCpuInfoSensor.hpp"

namespace ny::infra::windows::sensor {

    void WindowsCpuInfoSensor::update() {
        const auto raw = m_reader.read();
        m_name = raw.name;
        m_coreCount = raw.coreCount;
        m_threadCount = raw.threadCount;
    }

    const std::string& WindowsCpuInfoSensor::readName() const {
        return m_name;
    }

    int WindowsCpuInfoSensor::readCoreCount() const {
        return m_coreCount;
    }

    int WindowsCpuInfoSensor::readThreadCount() const {
        return m_threadCount;
    }

} // namespace ny::infra::windows::sensor
