// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: ThermGuardIoctls.hpp
 * Source File Name: ThermGuardCpuReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 16/02/2026
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERMGUARD_DEVICE_TYPE 0x8000

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#define METHOD_BUFFERED  0
#define FILE_READ_DATA   1

#define IOCTL_THERMGUARD_QUERY_CPU_INFO \
    CTL_CODE(THERMGUARD_DEVICE_TYPE, 0x800, METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_THERMGUARD_QUERY_TEMPERATURE \
    CTL_CODE(THERMGUARD_DEVICE_TYPE, 0x801, METHOD_BUFFERED, FILE_READ_DATA)

#pragma pack(push, 1)

    typedef struct _THERMGUARD_CPU_INFO {
        char     BrandString[64];
        uint32_t CurrentMHz;
        int32_t  TemperatureC;
        uint32_t PowerMilliWatts;
    } THERMGUARD_CPU_INFO;

    typedef struct _CPU_TEMPERATURE {
        uint32_t ProcessorIndex;
        int32_t  TemperatureCelsius;
        uint32_t IsValid;
    } CPU_TEMPERATURE;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
