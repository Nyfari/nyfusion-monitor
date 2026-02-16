// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: LogicalProcessorReader.hpp
 * Source File Name: LogicalProcessorReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "LogicalProcessorReader.hpp"

#include <windows.h>
#include <vector>
#include <bitset>

namespace ny::infra::windows::reader {

    std::optional<std::pair<int, int>>
        LogicalProcessorReader::readCoreAndThreadCount() const {

        DWORD length = 0;

        if (!GetLogicalProcessorInformationEx(RelationProcessorCore,
            nullptr,
            &length) &&
            GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return std::nullopt;
        }

        std::vector<uint8_t> buffer(length);

        if (!GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                buffer.data()),
            &length)) {
            return std::nullopt;
        }

        int coreCount = 0;
        int logicalCount = 0;

        DWORD offset = 0;

        while (offset < length) {

            auto* info =
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                    buffer.data() + offset);

            if (info->Relationship == RelationProcessorCore) {

                coreCount++;

                KAFFINITY mask = info->Processor.GroupMask[0].Mask;

                logicalCount += static_cast<int>(
                    std::bitset<sizeof(KAFFINITY) * 8>(mask).count());
            }

            offset += info->Size;
        }

        if (coreCount == 0 || logicalCount == 0) {
            return std::nullopt;
        }

        return std::make_pair(coreCount, logicalCount);
    }

}