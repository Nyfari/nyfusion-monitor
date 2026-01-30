/**
 * @file LinuxMemoryProvider.cpp
 * Created by
 * @author Marcos Henrique
 * @date 04/01/2026
 */
#include "LinuxMemoryProvider.hpp"
#include "linux/sensors/ram/LinuxMemorySensor.hpp"
#include "common/IMemorySensor.hpp"

namespace ny::infra::linux
{
    std::vector<std::unique_ptr<ny::infra::common::ISensor>>
    LinuxMemoryProvider::initSensors() const
    {
        std::vector<std::unique_ptr<ny::infra::common::ISensor>> sensors;
        sensors.push_back(std::make_unique<sensor::LinuxMemorySensor>());
        return sensors;
    }

    ny::domain::hardware::MemoryInfo LinuxMemoryProvider::collect() const {
        using namespace ny::infra::common;
        using namespace ny::domain::hardware;

        // Cria MemoryInfo com totalBytes 0 (ou você pode inicializar com memSensor->readTotalMemory() direto)
        MemoryInfo info;

        // Inicializa sensores
        auto sensors = initSensors();
        for (auto& sensor : sensors)
            sensor->update();

        // Lê o sensor de memória
        auto memSensor = dynamic_cast<IMemorySensor*>(sensors[0].get());
        if (memSensor) {
            info = MemoryInfo(memSensor->readTotalMemory()); // seta total
            info.setUsedBytes(memSensor->readUsedMemory());
            info.setFreeBytes(memSensor->readFreeMemory());
            info.setUsagePercent(memSensor->readUsagePercent());
        }

        return info;
    }
}
