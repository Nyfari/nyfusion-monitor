// include/shared.h
#pragma once

#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
  // CTL_CODE e tipos IOCTL são definidos em winioctl.h
#include <winioctl.h>
#endif

// Se por algum motivo CTL_CODE não estiver definido, evita erro de compilação
#ifndef CTL_CODE
#define FILE_DEVICE_UNKNOWN 0x00000022
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0
#define CTL_CODE(DeviceType, Function, Method, Access) \
      (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#define FILE_DEVICE_NYCPU 0x8000
#define IOCTL_READ_CPU_TEMPERATURE CTL_CODE(FILE_DEVICE_NYCPU, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

// Request: coreIndex = -1 => package; 0..N => core index
struct CPU_TEMP_REQUEST {
    int32_t coreIndex;
};

// Single entry result
struct CPU_TEMP_ENTRY {
    int32_t coreIndex;    // -1 = package
    uint8_t success;      // 1 = ok, 0 = invalid
    double temperatureC;  // Celsius
};

// Flexible result: allocate buffer large enough for entryCount entries
struct CPU_TEMP_RESULT {
    uint32_t entryCount;
    CPU_TEMP_ENTRY entries[1];
};
