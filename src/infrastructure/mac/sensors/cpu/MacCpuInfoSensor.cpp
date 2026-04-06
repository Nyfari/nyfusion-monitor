/**
 * @file MacCpuInfoSensor.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "MacCpuInfoSensor.hpp"

namespace ny::infra::mac::sensor {

    MacCpuInfoSensor::MacCpuInfoSensor(ny::infra::mac::reader::MacCpuInfoReader reader)
        : m_reader(std::move(reader)) {
    }

    void MacCpuInfoSensor::update() {
        const auto raw = m_reader.read();
        m_name = raw.name;
        m_coreCount = raw.coreCount;
        m_threadCount = raw.threadCount;
    }

    const std::string& MacCpuInfoSensor::readName() const {
        return m_name;
    }

    int MacCpuInfoSensor::readCoreCount() const {
        return m_coreCount;
    }

    int MacCpuInfoSensor::readThreadCount() const {
        return m_threadCount;
    }

} // namespace ny::infra::mac::sensor
