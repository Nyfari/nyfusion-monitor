#pragma once
/**
 * @file LinuxCpuInfoSensor.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_LINUXCPUINFOSENSOR_HPP
#define NY_FUSION_MONITOR_LINUXCPUINFOSENSOR_HPP

#include <string>
#include <utility>

#include "common/ISensor.hpp"
#include "linux/readers/LinuxCpuInfoReader.hpp"

namespace ny::infra::linux::sensor {

    class LinuxCpuInfoSensor final : public ny::infra::common::ISensor {
    public:
        explicit LinuxCpuInfoSensor(
            ny::infra::linux::reader::LinuxCpuInfoReader reader = {}
        )
            : m_reader(std::move(reader)) {}

        void update() override {
            m_name = m_reader.readCpuName();
            m_coreCount = m_reader.readCoreCount();
            m_threadCount = m_reader.readThreadCount();
        }

        [[nodiscard]] const std::string& readName() const {
            return m_name;
        }

        [[nodiscard]] int readCoreCount() const {
            return m_coreCount;
        }

        [[nodiscard]] int readThreadCount() const {
            return m_threadCount;
        }

    private:
        ny::infra::linux::reader::LinuxCpuInfoReader m_reader;
        std::string m_name{};
        int m_coreCount{0};
        int m_threadCount{0};
    };

} // namespace ny::infra::linux::sensor

#endif // NY_FUSION_MONITOR_LINUXCPUINFOSENSOR_HPP
