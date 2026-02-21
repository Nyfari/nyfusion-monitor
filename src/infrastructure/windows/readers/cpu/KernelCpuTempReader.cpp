// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: KernelCpuTempReader.hpp
 * Source File Name: KernelCpuTempReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 16/02/2026
 */
#include "KernelCpuTempReader.hpp"
#include "../include/ny/log/Logger.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>

// shared.h deve estar no include path (include/shared.h)
#include "../include/shared.h"

namespace ny::infra::windows::reader {

    std::optional<double> KernelCpuTempReader::readTemperatureCelsius(int coreIndex) {
        using ny::log::Logger;
        Logger::log(ny::log::Level::Debug, "KernelCpuTempReader", "Attempting to open device \\\\.\\NyCpuTemp");

        constexpr wchar_t DEVICE_NAME[] = L"\\\\.\\NyCpuTemp";
        HANDLE h = CreateFileW(DEVICE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            Logger::log(ny::log::Level::Warn, "KernelCpuTempReader",
                "CreateFileW failed. GetLastError=" + std::to_string(err));
            return std::nullopt;
        }

        CPU_TEMP_REQUEST req{};
        req.coreIndex = static_cast<int32_t>(coreIndex);

        const DWORD outSize = static_cast<DWORD>(sizeof(CPU_TEMP_RESULT) + (8 - 1) * sizeof(CPU_TEMP_ENTRY));
        std::vector<uint8_t> outBuf(outSize);
        DWORD bytesReturned = 0;

        BOOL ok = DeviceIoControl(h,
            IOCTL_READ_CPU_TEMPERATURE,
            &req, sizeof(req),
            outBuf.data(), outSize,
            &bytesReturned, nullptr);
        DWORD dioErr = ok ? 0 : GetLastError();
        Logger::log(ny::log::Level::Debug, "KernelCpuTempReader",
            "DeviceIoControl ok=" + std::to_string(ok) +
            " dioErr=" + std::to_string(dioErr) +
            " bytesReturned=" + std::to_string(bytesReturned));

        if (!ok || bytesReturned < sizeof(uint32_t)) {
            CloseHandle(h);
            Logger::log(ny::log::Level::Warn, "KernelCpuTempReader", "DeviceIoControl failed or returned too few bytes");
            return std::nullopt;
        }

        CPU_TEMP_RESULT* res = reinterpret_cast<CPU_TEMP_RESULT*>(outBuf.data());
        Logger::log(ny::log::Level::Debug, "KernelCpuTempReader", "entryCount=" + std::to_string(res->entryCount));
        if (res->entryCount == 0) {
            CloseHandle(h);
            Logger::log(ny::log::Level::Warn, "KernelCpuTempReader", "Driver returned entryCount == 0");
            return std::nullopt;
        }

        const CPU_TEMP_ENTRY& e = res->entries[0];
        Logger::log(ny::log::Level::Info, "KernelCpuTempReader",
            "entry0 coreIndex=" + std::to_string(e.coreIndex) +
            " success=" + std::to_string(e.success) +
            " temp=" + std::to_string(e.temperatureC));

        CloseHandle(h);
        if (!e.success) return std::nullopt;
        return e.temperatureC;
    }

} // namespace ny::infra::windows::reader
