// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file WindowsCpuInfoSensor.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#ifndef NY_FUSION_MONITOR_WINDOWSCPUINFOSENSOR_HPP
#define NY_FUSION_MONITOR_WINDOWSCPUINFOSENSOR_HPP

#include <string>
#include <utility>

#include "common/ISensor.hpp"
#include "windows/readers/cpu/WindowsCpuInfoReader.hpp"

namespace ny::infra::windows::sensor {

    class WindowsCpuInfoSensor final : public ny::infra::common::ISensor {
    public:
        explicit WindowsCpuInfoSensor(
            ny::infra::windows::reader::WindowsCpuInfoReader reader = {}
        )
            : m_reader(std::move(reader)) {}

        void update() override;

        [[nodiscard]] const std::string& readName() const;
        [[nodiscard]] int readCoreCount() const;
        [[nodiscard]] int readThreadCount() const;

    private:
        ny::infra::windows::reader::WindowsCpuInfoReader m_reader;
        std::string m_name{};
        int m_coreCount{0};
        int m_threadCount{0};
    };

} // namespace ny::infra::windows::sensor

#endif // NY_FUSION_MONITOR_WINDOWSCPUINFOSENSOR_HPP
