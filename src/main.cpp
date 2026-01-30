#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip> // std::setprecision
#include <cstdlib> // std::system

int main() {
    ny::infra::linux::LinuxCPUProvider provider;

    constexpr auto updateInterval = std::chrono::milliseconds(500);
    constexpr auto totalDuration = std::chrono::seconds(30);
    auto startTime = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - startTime < totalDuration) {
        // Limpa o console
#if defined(_WIN32) || defined(_WIN64)
        std::system("cls");
#else
        std::system("clear");
#endif

        auto cpuInfo = provider.collect();

        std::cout << "=== CPU Info ===\n";
        std::cout << "Nome: " << cpuInfo.name << "\n";
        std::cout << "Cores: " << cpuInfo.coreCount << "\n";
        std::cout << "Threads: " << cpuInfo.threadCount << "\n";
        std::cout << "Temperatura média: "
                  << static_cast<int>(cpuInfo.temperatureCelsius) << " °C\n";
        std::cout << "Potência: "
                  << (cpuInfo.powerWatts.has_value() ?
                      std::to_string(cpuInfo.powerWatts.value()) + " W" :
                      "Não disponível") << "\n\n";

        std::cout << "=== Threads ===\n";
        for (const auto& thread : cpuInfo.threads) {
            std::cout << "Thread " << thread.threadId
                      << " | Frequência: " << std::fixed << std::setprecision(2)
                      << thread.frequencyMHz << " MHz"
                      << " | Uso: " << std::fixed << std::setprecision(2)
                      << thread.usagePercent << " %\n";
        }

        double freqAvg = cpuInfo.averageFrequencyMHz;
        double usageAvg = cpuInfo.usagePercent;

        std::cout << "\nFrequência média: " << std::fixed << std::setprecision(2)
                  << freqAvg << " MHz\n";
        std::cout << "Uso médio total: " << std::fixed << std::setprecision(2)
                  << usageAvg << " %\n";

        std::this_thread::sleep_for(updateInterval);
    }

    return 0;
}
