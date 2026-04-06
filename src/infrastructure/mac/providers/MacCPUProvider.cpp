/**
 * @file MacCPUProvider.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "MacCPUProvider.hpp"

#include <algorithm>
#include <cstddef>

namespace ny::infra::mac {

    MacCPUProvider::MacCPUProvider(
        std::unique_ptr<ny::infra::mac::sensor::MacCpuInfoSensor> infoSensor,
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
    )
        : m_infoSensor(std::move(infoSensor))
        , m_frequencySensor(std::move(frequencySensor))
        , m_usageSensor(std::move(usageSensor))
        , m_temperatureSensor(std::move(temperatureSensor)) {
    }

    ny::domain::hardware::CPUInfo MacCPUProvider::collect() {
        using namespace ny::domain::hardware;

        CPUInfo info{};

        m_infoSensor->update();
        m_frequencySensor->update();
        m_usageSensor->update();
        m_temperatureSensor->update();

        info.name = m_infoSensor->readName();
        info.coreCount = m_infoSensor->readCoreCount();
        info.threadCount = m_infoSensor->readThreadCount();
        info.temperatureCelsius = m_temperatureSensor->readAverageTemperatureCelsius();
        info.averageFrequencyMHz = m_frequencySensor->readAverageFrequencyMHz();
        info.usagePercent = m_usageSensor->readTotalUsagePercent();
        info.powerWatts = std::nullopt;

        const auto frequencies = m_frequencySensor->readPerThreadFrequencyMHz();
        const auto usage = m_usageSensor->readPerThreadUsagePercent();

        int threads = info.threadCount;
        threads = std::min(threads, static_cast<int>(frequencies.size()));
        threads = std::min(threads, static_cast<int>(usage.size()));
        if (threads < 0) {
            threads = 0;
        }

        info.threads.reserve(static_cast<std::size_t>(threads));
        for (int i = 0; i < threads; ++i) {
            CPUThreadInfo threadInfo;
            threadInfo.threadId = i;
            threadInfo.frequencyMHz = frequencies[static_cast<std::size_t>(i)];
            threadInfo.usagePercent = usage[static_cast<std::size_t>(i)];
            info.threads.push_back(threadInfo);
        }

        return info;
    }

} // namespace ny::infra::mac
