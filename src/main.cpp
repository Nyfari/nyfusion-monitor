#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdlib>

#if defined(_WIN32)
#include "infrastructure/windows/providers/WindowsCpuProvider.hpp"
#include "infrastructure/windows/providers/WindowsMemoryProvider.hpp"
namespace platform = ny::infra::windows;

#elif defined(__linux__)
#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include "infrastructure/linux/providers/LinuxMemoryProvider.hpp"
namespace platform = ny::infra::linux;

#else
#error Unsupported platform
#endif

using namespace ny::domain::hardware;

int main()
{
#if defined(_WIN32)
    platform::WindowsCPUProvider cpuProvider;
    platform::WindowsMemoryProvider memProvider;
#elif defined(__linux__)
    platform::LinuxCPUProvider cpuProvider;
    platform::LinuxMemoryProvider memProvider;
#endif

    constexpr int updateIntervalMs = 1000;
    constexpr int totalDurationMs = 30'000;
    const int iterations = totalDurationMs / updateIntervalMs;

    for (int i = 0; i < iterations; ++i)
    {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif

        CPUInfo cpuInfo = cpuProvider.collect();
        MemoryInfo memInfo = memProvider.collect();

        std::cout << "=== CPU Info ===\n";
        std::cout << "Nome: " << cpuInfo.name << "\n";
        std::cout << "Cores: " << cpuInfo.coreCount << "\n";
        std::cout << "Threads: " << cpuInfo.threadCount << "\n";
        std::cout << "Temperatura media: "
            << static_cast<int>(cpuInfo.temperatureCelsius) << " C\n";

        std::cout << "Potencia: "
            << (cpuInfo.powerWatts
                ? std::to_string(*cpuInfo.powerWatts) + " W"
                : "Nao disponivel") << "\n\n";

        std::cout << "Frequencia media: "
            << std::fixed << std::setprecision(2)
            << cpuInfo.averageFrequencyMHz << " MHz\n";

        std::cout << "Uso medio total: "
            << std::fixed << std::setprecision(2)
            << cpuInfo.usagePercent << " %\n\n";

        std::cout << "=== Memoria RAM ===\n";
        std::cout << "Total: " << memInfo.totalBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Usada: " << memInfo.usedBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Livre: " << memInfo.freeBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Uso: "
            << std::fixed << std::setprecision(2)
            << memInfo.usagePercent() << " %\n";

        std::this_thread::sleep_for(
            std::chrono::milliseconds(updateIntervalMs));
    }

    return 0;
}
