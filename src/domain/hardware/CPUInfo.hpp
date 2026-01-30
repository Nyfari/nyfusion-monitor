#pragma once
/**
 * @file CPUInfo.hpp
 * Created by
 * @author Marcos Henrique
 * @date 14/12/2025
 *
 * @brief
 */
#include <string>
#include <vector>
#include <optional>

namespace ny::domain::hardware {

    struct CPUThreadInfo {
        int threadId;
        double frequencyMHz;
        double usagePercent;
    };

    struct CPUInfo {
        std::string name;
        double temperatureCelsius;
        double averageFrequencyMHz;
        double usagePercent;
        int coreCount;
        int threadCount;
        std::vector<CPUThreadInfo> threads;
        std::optional<double> powerWatts;
    };

}