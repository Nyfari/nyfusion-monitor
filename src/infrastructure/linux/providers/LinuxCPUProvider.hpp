#pragma once
/**
 * @file LinuxCPUProvider.hpp
 * @author Marcos Henrique
 * @date 04/01/2026
 *
 * @brief Fornece informações de CPU no Linux usando sensores específicos.
 */

#include "../domain/providers/CPUProvider.hpp"
#include "common/ISensor.hpp"

#include <memory>
#include <vector>
#include <set>

namespace ny::infra::linux {

    class LinuxCPUProvider final : public ny::domain::providers::CPUProvider {
    public:
        ny::domain::hardware::CPUInfo collect() override;

    private:
        // ──────────────
        // Helpers internos
        // ──────────────
        static std::string readCpuName();
        static int countCores();
        static int countThreads();

        // Sensores
        std::vector<std::unique_ptr<common::ISensor>> initSensors() const;
    };

} // namespace ny::infra::linux
