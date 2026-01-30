#include <cmath>

#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include <iostream>
#include <iomanip>

int main() {
    ny::infra::linux::LinuxCPUProvider provider;
    auto info = provider.collect();

    std::cout << "=== CPU Info ===\n";
    std::cout << "Nome: " << info.name << "\n";
    std::cout << "Cores: " << info.coreCount << "\n";
    std::cout << "Threads: " << info.threadCount << "\n";
    std::cout << "Temperatura média: " << std::round(info.temperatureCelsius) << " °C\n";
    std::cout << "Potência: " << (info.powerWatts ? std::to_string(*info.powerWatts) + " W" : "Não disponível") << "\n\n";

    std::cout << "=== Threads ===\n";
    for (auto& t : info.threads) {
        std::cout << "Thread " << t.threadId
                  << " | Frequência: " << std::fixed << std::setprecision(2) << t.frequencyMHz << " MHz"
                  << " | Uso: " << std::fixed << std::setprecision(2) << t.usagePercent << " %\n";
    }

    std::cout << "\nFrequência média: " << std::fixed << std::setprecision(2) << info.averageFrequencyMHz << " MHz\n";
    std::cout << "Uso médio total: " << std::fixed << std::setprecision(2) << info.usagePercent << " %\n";

    return 0;
}