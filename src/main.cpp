// main.cpp
// NyFusion Monitor - exemplo de uso dos providers e inicialização do logger
// Autor: Marcos Henrique
// Data: 12/02/2026

#include "include/ny/log/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <filesystem>

#if defined(_WIN32)
#include "infrastructure/windows/providers/WindowsCPUProvider.hpp"
#include "infrastructure/windows/providers/WindowsMemoryProvider.hpp"
namespace platform = ny::infra::windows;
#elif defined(__linux__)
#include "infrastructure/linux/providers/LinuxCPUProvider.hpp"
#include "infrastructure/linux/providers/LinuxMemoryProvider.hpp"
namespace platform = ny::infra::linux;
#else
#error Unsupported platform
#endif
#include <include/ny/log/Logger.hpp>

using namespace ny::domain::hardware;

int main() {
    // Inicializa logger (arquivo em %TEMP%/nyfusion-monitor.log)
    std::filesystem::path logPath = std::filesystem::temp_directory_path() / "nyfusion-monitor.log";
    ny::log::Logger::init(logPath.string());
    ny::log::Logger::log(ny::log::Level::Info, "App", "NyFusion Monitor starting. Log: " + logPath.string());

    // Cria providers
    platform::WindowsCPUProvider cpuProvider;
    platform::WindowsMemoryProvider memProvider;

    constexpr int updateIntervalMs = 1000;
    constexpr int totalDurationMs = 30'000;
    const int iterations = totalDurationMs / updateIntervalMs;

    for (int i = 0; i < iterations; ++i) {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif

        // Coleta dados
        CPUInfo cpuInfo = cpuProvider.collect();
        MemoryInfo memInfo = memProvider.collect();

        // Log resumo rápido
        ny::log::Logger::log(ny::log::Level::Debug, "App",
            "CPU temp (avg) = " + std::to_string(cpuInfo.temperatureCelsius) +
            " C, freq avg = " + std::to_string(cpuInfo.averageFrequencyMHz) +
            " MHz, usage = " + std::to_string(cpuInfo.usagePercent) + " %");

        // Imprime no console
        std::cout << "=== CPU Info ===\n";
        std::cout << "Nome: " << cpuInfo.name << "\n";
        std::cout << "Cores: " << cpuInfo.coreCount << "\n";
        std::cout << "Threads: " << cpuInfo.threadCount << "\n";
        // Se temperatura for 0 e você quiser distinguir "sem leitura", considere usar optional no futuro
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
            << memInfo.usagePercent() << " %\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs));
    }

    ny::log::Logger::log(ny::log::Level::Info, "App", "NyFusion Monitor exiting");
    ny::log::Logger::shutdown();
    return 0;
}
