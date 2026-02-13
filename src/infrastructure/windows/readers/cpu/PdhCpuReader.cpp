// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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
        PdhOpenQuery(nullptr, 0, &query);
        PdhAddEnglishCounter(
            query,
            L"\\Processor(*)\\% Processor Time",
            0,
            &counter
        );
        PdhCollectQueryData(query);
    }

    PdhCpuReader::~PdhCpuReader() {
        if (query)
            PdhCloseQuery(query);
    }

    std::vector<double> PdhCpuReader::readPerLogicalProcessorUsage() {
        PdhCollectQueryData(query);

        DWORD bufferSize = 0;
        DWORD itemCount = 0;

        PdhGetFormattedCounterArray(
            counter,
            PDH_FMT_DOUBLE,
            &bufferSize,
            &itemCount,
            nullptr
        );

        std::vector<uint8_t> buffer(bufferSize);

        auto* items =
            reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM*>(buffer.data());

        PdhGetFormattedCounterArray(
            counter,
            PDH_FMT_DOUBLE,
            &bufferSize,
            &itemCount,
            items
        );

        std::vector<double> result;
        result.reserve(itemCount);

        for (DWORD i = 0; i < itemCount; ++i)
            result.push_back(items[i].FmtValue.doubleValue);

        return result;
    }
}