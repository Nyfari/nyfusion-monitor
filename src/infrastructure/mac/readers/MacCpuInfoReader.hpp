#pragma once
/**
 * @file MacCpuInfoReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_MACCPUINFOREADER_HPP
#define NY_FUSION_MONITOR_MACCPUINFOREADER_HPP

#include <string>

namespace ny::infra::mac::reader {

    struct MacCpuInfoRaw final {
        std::string name{};
        int coreCount{0};
        int threadCount{0};
    };

    class MacCpuInfoReader final {
    public:
        [[nodiscard]] MacCpuInfoRaw read() const;
    };

} // namespace ny::infra::mac::reader

#endif // NY_FUSION_MONITOR_MACCPUINFOREADER_HPP
