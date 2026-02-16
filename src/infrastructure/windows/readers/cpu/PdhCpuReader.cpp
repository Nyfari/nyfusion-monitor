// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: PdhCpuReader.hpp
 * Source File Name: PdhCpuReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "PdhCpuReader.hpp"

#include <stdexcept>

namespace ny::infra::windows::reader {

    PdhCpuReader::PdhCpuReader() {
        if (PdhOpenQuery(nullptr, 0, &query_) != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to open PDH query");
        }

        if (PdhAddEnglishCounter(
            query_,
            "\\Processor(_Total)\\% Processor Time",
            0,
            &counter_) != ERROR_SUCCESS) {
            PdhCloseQuery(query_);
            throw std::runtime_error("Failed to add PDH counter");
        }

        PdhCollectQueryData(query_);
    }

    PdhCpuReader::~PdhCpuReader() {
        if (query_) {
            PdhCloseQuery(query_);
        }
    }

    double PdhCpuReader::readTotalUsage() const {
        if (PdhCollectQueryData(query_) != ERROR_SUCCESS) {
            return 0.0;
        }

        PDH_FMT_COUNTERVALUE value{};
        if (PdhGetFormattedCounterValue(
            counter_,
            PDH_FMT_DOUBLE,
            nullptr,
            &value) != ERROR_SUCCESS) {
            return 0.0;
        }

        return value.doubleValue;
    }

}