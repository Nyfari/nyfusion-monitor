#pragma once
/**
 * @file HardwareBackendService.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_HARDWAREBACKENDSERVICE_HPP
#define NY_FUSION_MONITOR_HARDWAREBACKENDSERVICE_HPP

#include <memory>
#include <optional>
#include <string>

#include "services/HardwareMonitorService.hpp"
#include "hardware/MemoryInfo.hpp"
#include "providers/CPUProvider.hpp"
#include "providers/GPUProvider.hpp"
#include "providers/MemoryProvider.hpp"

namespace ny::ui::services {

    class HardwareBackendService final {
    public:
        struct DashboardSnapshot final {
            ny::domain::hardware::CPUInfo cpu;
            ny::domain::hardware::GPUInfo gpu;
            ny::domain::hardware::MemoryInfo memory;
        };

        HardwareBackendService();

        [[nodiscard]] std::optional<DashboardSnapshot> readSnapshot();
        [[nodiscard]] const std::string& lastError() const;

    private:
        std::unique_ptr<ny::domain::providers::CPUProvider> m_cpuProvider;
        std::unique_ptr<ny::domain::providers::MemoryProvider> m_memoryProvider;
        std::unique_ptr<ny::domain::providers::GPUProvider> m_gpuProvider;
        std::unique_ptr<ny::application::services::HardwareMonitorService> m_monitorService;
        std::string m_lastError{};
    };

} // namespace ny::ui::services

#endif // NY_FUSION_MONITOR_HARDWAREBACKENDSERVICE_HPP
