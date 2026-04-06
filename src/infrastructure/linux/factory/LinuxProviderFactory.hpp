#pragma once

#include <memory>

namespace ny::domain::providers {
    class CPUProvider;
    class MemoryProvider;
    class GPUProvider;
}

namespace ny::infra::linux {

    class LinuxProviderFactory final {
    public:
        [[nodiscard]] std::unique_ptr<ny::domain::providers::CPUProvider>
        createCpuProvider() const;

        [[nodiscard]] std::unique_ptr<ny::domain::providers::MemoryProvider>
        createMemoryProvider() const;

        [[nodiscard]] std::unique_ptr<ny::domain::providers::GPUProvider>
        createGpuProvider() const;
    };

} // namespace ny::infra::linux
