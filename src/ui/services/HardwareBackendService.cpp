/**
 * @file HardwareBackendService.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "HardwareBackendService.hpp"

#include <exception>

#if defined(__linux__)
#include "linux/factory/LinuxProviderFactory.hpp"
using PlatformProviderFactory = ny::infra::linux::LinuxProviderFactory;
#else
#error Unsupported platform
#endif

namespace ny::ui::services {

    HardwareBackendService::HardwareBackendService() {
        PlatformProviderFactory providerFactory;
        m_cpuProvider = providerFactory.createCpuProvider();
        m_memoryProvider = providerFactory.createMemoryProvider();
        m_gpuProvider = providerFactory.createGpuProvider();

        m_monitorService = std::make_unique<ny::application::services::HardwareMonitorService>(
            *m_cpuProvider,
            *m_memoryProvider,
            *m_gpuProvider
        );
    }

    std::optional<HardwareBackendService::DashboardSnapshot> HardwareBackendService::readSnapshot() {
        try {
            const auto snapshot = m_monitorService->collectSnapshot();
            m_lastError.clear();
            return DashboardSnapshot{snapshot.cpu, snapshot.gpu, snapshot.memory};
        } catch (const std::exception& ex) {
            m_lastError = ex.what();
            return std::nullopt;
        } catch (...) {
            m_lastError = "Falha desconhecida ao coletar hardware";
            return std::nullopt;
        }
    }

    const std::string& HardwareBackendService::lastError() const {
        return m_lastError;
    }

} // namespace ny::ui::services
