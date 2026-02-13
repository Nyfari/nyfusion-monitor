// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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

namespace ny::infra::windows::reader {

    int LogicalProcessorReader::readLogicalProcessorCount() const {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return static_cast<int>(sysInfo.dwNumberOfProcessors);
    }

    int LogicalProcessorReader::readPhysicalCoreCount() const {

        DWORD len = 0;
        GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            nullptr,
            &len
        );

        std::vector<uint8_t> buffer(len);

        auto* ptr =
            reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                buffer.data()
            );

        GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            ptr,
            &len
        );

        int count = 0;

        while (reinterpret_cast<uint8_t*>(ptr) <
               buffer.data() + len) {

            if (ptr->Relationship == RelationProcessorCore)
                ++count;

            ptr = reinterpret_cast<
                PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
                reinterpret_cast<uint8_t*>(ptr) + ptr->Size
            );
               }

        return count;
    }
}