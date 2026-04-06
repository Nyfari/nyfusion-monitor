/**
 * @file MacCpuInfoReader.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "MacCpuInfoReader.hpp"

#include <sys/types.h>
#include <sys/sysctl.h>

#include <vector>

namespace ny::infra::mac::reader {

    static std::string readStringSysctl(const char* key) {
        size_t size = 0;
        if (sysctlbyname(key, nullptr, &size, nullptr, 0) != 0 || size == 0) {
            return {};
        }

        std::vector<char> buffer(size);
        if (sysctlbyname(key, buffer.data(), &size, nullptr, 0) != 0 || buffer.empty()) {
            return {};
        }

        return std::string(buffer.data());
    }

    static int readIntSysctl(const char* key) {
        int value = 0;
        size_t size = sizeof(value);
        if (sysctlbyname(key, &value, &size, nullptr, 0) != 0) {
            return 0;
        }
        return value;
    }

    MacCpuInfoRaw MacCpuInfoReader::read() const {
        MacCpuInfoRaw raw{};
        raw.name = readStringSysctl("machdep.cpu.brand_string");
        raw.coreCount = readIntSysctl("hw.physicalcpu");
        raw.threadCount = readIntSysctl("hw.logicalcpu");
        return raw;
    }

} // namespace ny::infra::mac::reader
