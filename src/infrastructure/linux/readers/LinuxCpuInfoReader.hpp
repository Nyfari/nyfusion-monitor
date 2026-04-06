#pragma once
/**
 * @file LinuxCpuInfoReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_LINUXCPUINFOREADER_HPP
#define NY_FUSION_MONITOR_LINUXCPUINFOREADER_HPP

#include <fstream>
#include <set>
#include <string>
#include <utility>

namespace ny::infra::linux::reader {

    class LinuxCpuInfoReader final {
    public:
        [[nodiscard]] std::string readCpuName() const {
            std::ifstream file("/proc/cpuinfo");
            std::string line;

            while (std::getline(file, line)) {
                if (line.starts_with("model name")) {
                    const auto pos = line.find(':');
                    if (pos != std::string::npos && pos + 2 <= line.size()) {
                        return line.substr(pos + 2);
                    }
                }
            }

            return "Unknown CPU";
        }

        [[nodiscard]] int readCoreCount() const {
            std::ifstream file("/proc/cpuinfo");
            std::string line;
            std::set<std::pair<int, int>> cores;

            int physicalId = -1;
            int coreId = -1;

            while (std::getline(file, line)) {
                if (line.starts_with("physical id")) {
                    const auto pos = line.find(':');
                    if (pos != std::string::npos) {
                        physicalId = std::stoi(line.substr(pos + 2));
                    }
                } else if (line.starts_with("core id")) {
                    const auto pos = line.find(':');
                    if (pos != std::string::npos) {
                        coreId = std::stoi(line.substr(pos + 2));
                    }
                } else if (line.empty() && physicalId != -1 && coreId != -1) {
                    cores.insert({physicalId, coreId});
                    physicalId = -1;
                    coreId = -1;
                }
            }

            return static_cast<int>(cores.size());
        }

        [[nodiscard]] int readThreadCount() const {
            std::ifstream file("/proc/cpuinfo");
            std::string line;
            int count = 0;

            while (std::getline(file, line)) {
                if (line.starts_with("processor")) {
                    ++count;
                }
            }

            return count;
        }
    };

} // namespace ny::infra::linux::reader

#endif // NY_FUSION_MONITOR_LINUXCPUINFOREADER_HPP
