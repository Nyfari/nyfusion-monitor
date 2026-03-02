#pragma once
/**
 * @file GPUInfo.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ny::domain::hardware
{
    struct GPUInfo {
        // Identificação
        std::string vendor;           // "AMD", "NVIDIA", "Intel"
        std::string model;            // "RADEON RX 7900 XTX", "RTX 4090", etc.

        // Memória (valores derivados para conveniência)
        std::uint64_t vramTotalBytes;
        std::uint32_t vramTotalMB;    // = vramTotalBytes / (1024 * 1024)
        std::uint32_t vramTotalGB;    // = vramTotalBytes / (1024 * 1024 * 1024)

        // Métricas opcionais (podem não estar disponíveis)
        std::optional<std::uint64_t> vramUsedMB;
        std::optional<std::uint64_t> vramUsedGB;
        std::optional<float> utilizationPercent;    // 0.0 - 100.0
        std::optional<float> temperatureCelsius;
        std::optional<std::uint32_t> frequencyMHz;
        std::optional<float> powerWatts;

        // Driver
        std::string driverVersion;                  // "22.40.1", etc.

        // Capabilities (extensível para futuro)
        std::vector<std::string> supportedFeatures; // ["Ray Tracing", "DLSS", etc.]
    };
}
