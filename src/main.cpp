// main.cpp
// NyFusion Monitor - exemplo de uso dos providers
// Autor: Marcos Henrique
// Data: 12/02/2026

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#include "infrastructure/windows/providers/WindowsCPUProvider.hpp"
#include "infrastructure/windows/providers/WindowsMemoryProvider.hpp"
#include "include/ny/log/Logger.hpp"
namespace platform = ny::infra::windows;
#elif defined(__linux__)
#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include "infrastructure/linux/providers/LinuxMemoryProvider.hpp"
#include "infrastructure/linux/providers/LinuxGPUProvider.hpp"
namespace platform = ny::infra::linux;
#else
#error Unsupported platform
#endif

using namespace ny::domain::hardware;

int main()
{
#ifdef _WIN32
    // Logger apenas em Windows por enquanto
    std::filesystem::path logPath = std::filesystem::temp_directory_path() / "nyfusion-monitor.log";
    ny::log::Logger::init(logPath.string());
    ny::log::Logger::log(ny::log::Level::Info, "App", "NyFusion Monitor starting on Windows");
#else
    std::cout << "NyFusion Monitor starting on Linux\n";
#endif

    // Cria providers
#ifdef _WIN32
    platform::WindowsCPUProvider cpuProvider;
    platform::WindowsMemoryProvider memProvider;
#else
    platform::LinuxCPUProvider cpuProvider;
    platform::LinuxMemoryProvider memProvider;
    platform::LinuxGPUProvider gpuProvider;
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

        // Coleta dados
        CPUInfo cpuInfo = cpuProvider.collect();
        MemoryInfo memInfo = memProvider.collect();

        // Imprime no console
        std::cout << "=== CPU Info ===\n";
        std::cout << "Nome: " << cpuInfo.name << "\n";
        std::cout << "Cores: " << cpuInfo.coreCount << "\n";
        std::cout << "Threads: " << cpuInfo.threadCount << "\n";
        std::cout << "Temperatura media: " << static_cast<int>(cpuInfo.temperatureCelsius) << " C\n";

        std::cout << "Potencia: "
            << (cpuInfo.powerWatts ? std::to_string(*cpuInfo.powerWatts) + " W" : "Nao disponivel") << "\n\n";

        std::cout << "Frequencia media: " << std::fixed << std::setprecision(2)
            << cpuInfo.averageFrequencyMHz << " MHz\n";

        std::cout << "Uso medio total: " << std::fixed << std::setprecision(2)
            << cpuInfo.usagePercent << " %\n\n";

        std::cout << "=== Memoria RAM ===\n";
        std::cout << "Total: " << memInfo.totalBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Usada: " << memInfo.usedBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Livre: " << memInfo.freeBytes() / 1024 / 1024 << " MB\n";
        std::cout << "Uso: " << std::fixed << std::setprecision(2)
            << memInfo.usagePercent() << " %\n\n";

#ifdef __linux__
        // GPU Info (apenas Linux)
        GPUInfo gpuInfo = gpuProvider.collect();

        if (!gpuInfo.vendor.empty()) {
            std::cout << "=== GPU Info ===\n";
            std::cout << "Vendor: " << gpuInfo.vendor << "\n";
            std::cout << "Modelo: " << gpuInfo.model << "\n";
            std::cout << "VRAM Total: " << gpuInfo.vramTotalGB << " GB ("
                      << gpuInfo.vramTotalMB << " MB)\n";

            if (gpuInfo.vramUsedMB) {
                std::cout << "VRAM Usada: " << *gpuInfo.vramUsedMB << " MB";
                if (gpuInfo.vramUsedGB) {
                    std::cout << " (" << *gpuInfo.vramUsedGB << " GB)";
                }
                std::cout << "\n";
            }

            if (gpuInfo.utilizationPercent) {
                std::cout << "Utilizacao: " << std::fixed << std::setprecision(2)
                          << *gpuInfo.utilizationPercent << " %\n";
            }

            if (gpuInfo.temperatureCelsius) {
                std::cout << "Temperatura: " << std::fixed << std::setprecision(1)
                          << *gpuInfo.temperatureCelsius << " C\n";
            }

            if (gpuInfo.frequencyMHz) {
                std::cout << "Frequencia: " << *gpuInfo.frequencyMHz << " MHz\n";
            }

            if (gpuInfo.powerWatts) {
                std::cout << "Potencia: " << std::fixed << std::setprecision(2)
                          << *gpuInfo.powerWatts << " W\n";
            }

            std::cout << "Driver: " << gpuInfo.driverVersion << "\n";

            if (!gpuInfo.supportedFeatures.empty()) {
                std::cout << "Features: ";
                for (size_t j = 0; j < gpuInfo.supportedFeatures.size(); ++j) {
                    if (j > 0) std::cout << ", ";
                    std::cout << gpuInfo.supportedFeatures[j];
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        } else {
            std::cout << "=== GPU Info ===\n";
            std::cout << "Nenhuma GPU AMD detectada\n\n";
        }
#endif

        std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs));
    }

#ifdef _WIN32
    ny::log::Logger::log(ny::log::Level::Info, "App", "NyFusion Monitor exiting");
    ny::log::Logger::shutdown();
#else
    std::cout << "NyFusion Monitor exiting\n";
#endif

    return 0;
}