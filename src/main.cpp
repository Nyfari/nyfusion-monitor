#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdlib> // std::system("clear")

// Providers
#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include "infrastructure/linux/providers/LinuxMemoryProvider.hpp"

// Namespaces curtos
using namespace ny::infra::linux;
using namespace ny::domain::hardware;

int main() {
    LinuxCPUProvider cpuProvider;
    LinuxMemoryProvider memProvider;

    constexpr int updateIntervalMs = 1000;
    constexpr int totalDurationMs = 30'000;
    const int iterations = totalDurationMs / updateIntervalMs;

    for (int i = 0; i < iterations; ++i) {
        // Limpa o console
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif

        // Coleta informações
        CPUInfo cpuInfo = cpuProvider.collect();
        MemoryInfo memInfo = memProvider.collect();

        // ────────────── CPU ──────────────
        std::cout << "=== CPU Info ===\n";
        std::cout << "Nome: " << cpuInfo.name << "\n";
        std::cout << "Cores: " << cpuInfo.coreCount << "\n";
        std::cout << "Threads: " << cpuInfo.threadCount << "\n";
        std::cout << "Temperatura média: " << static_cast<int>(cpuInfo.temperatureCelsius) << " °C\n";
        std::cout << "Potência: " << (cpuInfo.powerWatts ? std::to_string(*cpuInfo.powerWatts) + " W" : "Não disponível") << "\n\n";

        std::cout << "=== Threads ===\n";
        for (const auto& thread : cpuInfo.threads) {
            std::cout << "Thread " << thread.threadId
                      << " | Frequência: " << std::fixed << std::setprecision(2) << thread.frequencyMHz << " MHz"
                      << " | Uso: " << std::fixed << std::setprecision(2) << thread.usagePercent << " %\n";
        }

        std::cout << "\nFrequência média: " << std::fixed << std::setprecision(2) << cpuInfo.averageFrequencyMHz << " MHz\n";
        std::cout << "Uso médio total: " << std::fixed << std::setprecision(2) << cpuInfo.usagePercent << " %\n\n";

        // ────────────── MEMÓRIA ──────────────
        std::cout << "=== Memória RAM ===\n";
        std::cout << "Total: " << memInfo.totalBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Usada: " << memInfo.usedBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Livre: " << memInfo.freeBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Uso: " << std::fixed << std::setprecision(2) << memInfo.usagePercent() << " %\n";

        // Espera 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs));
    }

    return 0;
}