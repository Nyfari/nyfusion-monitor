/**
 * @file LinuxCPUProvider.cpp
 * @author Marcos Henrique
 * @date 04/01/2026
 */

#include "LinuxCPUProvider.hpp"

#include <algorithm>

namespace ny::infra::linux {

    LinuxCPUProvider::LinuxCPUProvider()
        : LinuxCPUProvider(
            std::make_unique<ny::infra::linux::sensor::LinuxCpuInfoSensor>(),
            std::make_unique<ny::infra::linux::sensor::LinuxCpuFrequencySensor>(),
            std::make_unique<ny::infra::linux::sensor::LinuxCpuUsageSensor>(),
            std::make_unique<ny::infra::linux::sensor::LinuxCpuTemperatureSensor>()
        ) {
    }

    LinuxCPUProvider::LinuxCPUProvider(
        std::unique_ptr<ny::infra::linux::sensor::LinuxCpuInfoSensor> infoSensor,
        std::unique_ptr<ny::infra::common::ICpuFrequencySensor> frequencySensor,
        std::unique_ptr<ny::infra::common::ICpuUsageSensor> usageSensor,
        std::unique_ptr<ny::infra::common::ICpuTemperatureSensor> temperatureSensor
    )
        : m_infoSensor(std::move(infoSensor))
        , m_frequencySensor(std::move(frequencySensor))
        , m_usageSensor(std::move(usageSensor))
        , m_temperatureSensor(std::move(temperatureSensor)) {
    }

    ny::domain::hardware::CPUInfo LinuxCPUProvider::collect() {
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

        const int threads = std::min({
            info.threadCount,
            static_cast<int>(frequencies.size()),
            static_cast<int>(usage.size())
        });

        info.threads.reserve(static_cast<std::size_t>(std::max(0, threads)));

        for (int i = 0; i < threads; ++i) {
            info.threads.push_back(CPUThreadInfo{
                .threadId = i,
                .frequencyMHz = frequencies[static_cast<std::size_t>(i)],
                .usagePercent = usage[static_cast<std::size_t>(i)]
            });
        }

        return info;
    }

} // namespace ny::infra::linux
