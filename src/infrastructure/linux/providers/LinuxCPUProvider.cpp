/**
 * @file LinuxCPUProvider.cpp
 * @author Marcos Henrique
 * @date 04/01/2026
 */

#include "LinuxCPUProvider.hpp"

// Sensores concretos
#include "../sensors/cpu/LinuxCpuFrequencySensor.hpp"
#include "linux/sensors/cpu/LinuxCpuUsageSensor.hpp"
#include "linux/sensors/cpu/LinuxCpuTemperatureSensor.hpp"

#include <fstream>
#include <string>
#include <numeric>
#include <algorithm>
#include <thread>

namespace ny::infra::linux {

    // ──────────────
    // Helpers estáticos
    // ──────────────

    std::string LinuxCPUProvider::readCpuName() {
        std::ifstream file("/proc/cpuinfo");
        std::string line;

        while (std::getline(file, line)) {
            if (line.starts_with("model name")) {
                auto pos = line.find(':');
                return line.substr(pos + 2);
            }
        }
        return "Unknown CPU";
    }

    int LinuxCPUProvider::countCores() {
        std::ifstream file("/proc/cpuinfo");
        std::string line;
        std::set<std::pair<int,int>> cores;

        int physicalId = -1;
        int coreId = -1;

        while (std::getline(file, line)) {
            if (line.starts_with("physical id")) {
                auto pos = line.find(':');
                physicalId = std::stoi(line.substr(pos + 2));
            }
            else if (line.starts_with("core id")) {
                auto pos = line.find(':');
                coreId = std::stoi(line.substr(pos + 2));
            }
            else if (line.empty() && physicalId != -1 && coreId != -1) {
                cores.insert({physicalId, coreId});
                physicalId = coreId = -1;
            }
        }

        return cores.size(); // número de cores físicos
    }


    int LinuxCPUProvider::countThreads() {
        std::ifstream file("/proc/cpuinfo");
        std::string line;
        int count = 0;

        while (std::getline(file, line)) {
            if (line.starts_with("processor"))
                ++count;
        }
        return count;
    }

    // ──────────────
    // Inicializa sensores
    // ──────────────

    std::vector<std::unique_ptr<ny::infra::common::ISensor>> LinuxCPUProvider::initSensors() const {
        std::vector<std::unique_ptr<ny::infra::common::ISensor>> sensors;

        // Cria sensores concretos que implementam ISensor
        sensors.push_back(std::make_unique<sensor::LinuxCpuFrequencySensor>());
        sensors.push_back(std::make_unique<sensor::LinuxCpuUsageSensor>());
        sensors.push_back(std::make_unique<sensor::LinuxCpuTemperatureSensor>());
        // Futuro: sensors.push_back(std::make_unique<sensor::LinuxCpuPowerSensor>());

        return sensors;
    }

    // ──────────────
    // Coleta informações da CPU
    // ──────────────

    ny::domain::hardware::CPUInfo LinuxCPUProvider::collect() {
        using namespace ny::domain::hardware;
        using namespace ny::infra::common;
        using namespace ny::infra::linux::sensor;

        CPUInfo info{};
        info.name = readCpuName();
        info.coreCount = countCores();
        info.threadCount = countThreads();

        // Inicializa sensores concretos
        LinuxCpuFrequencySensor freqSensor;
        LinuxCpuUsageSensor usageSensor;
        LinuxCpuTemperatureSensor tempSensor;

        // Atualiza sensores
        freqSensor.update();

        // Atualiza duas vezes para uso real
        usageSensor.update();           // primeira leitura inicializa prevPerCpu
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // espera
        usageSensor.update();           // segunda leitura gera delta real

        tempSensor.update();

        // Frequências e uso
        auto frequencies = freqSensor.readPerThreadFrequencyMHz();
        auto usage       = usageSensor.readPerThreadUsagePercent();
        info.temperatureCelsius = tempSensor.readAverageTemperatureCelsius();

        // Threads
        const int threads = std::min({
            info.threadCount,
            static_cast<int>(frequencies.size()),
            static_cast<int>(usage.size())
        });

        double freqSum = 0.0;
        double usageSum = 0.0;

        for (int i = 0; i < threads; ++i) {
            info.threads.push_back({
                .threadId = i,
                .frequencyMHz = frequencies[i],
                .usagePercent = usage[i]
            });

            freqSum += frequencies[i];
            usageSum += usage[i];
        }

        info.averageFrequencyMHz = threads > 0 ? freqSum / threads : 0.0;
        info.usagePercent        = threads > 0 ? usageSum / threads : 0.0;
        info.powerWatts           = std::nullopt;

        return info;
    }

} // namespace ny::infra::linux