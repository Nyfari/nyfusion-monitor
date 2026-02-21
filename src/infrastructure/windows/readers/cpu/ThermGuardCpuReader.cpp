// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: ThermGuardCpuReader.hpp, ThermGuardIoctls.hpp
 * Source File Name: ThermGuardCpuReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 16/02/2026
 */

#include "ThermGuardCpuReader.hpp"
#include "ThermGuardIoctls.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <sstream>
#include <vector>

#include "../include/ny/log/Logger.hpp"

namespace ny::infra::windows::reader {

    namespace {

        class ScopedHandle {
        public:
            explicit ScopedHandle(HANDLE h) : handle(h) {}
            ~ScopedHandle() {
                if (handle != INVALID_HANDLE_VALUE && handle != nullptr) {
                    CloseHandle(handle);
                }
            }

            HANDLE get() const { return handle; }
            bool valid() const { return handle != INVALID_HANDLE_VALUE && handle != nullptr; }

        private:
            HANDLE handle;
        };

        std::string trimNullTerminated(const char* buffer, size_t size)
        {
            size_t len = 0;
            while (len < size && buffer[len] != '\0')
                ++len;
            return std::string(buffer, len);
        }

    }

    std::optional<ThermGuardCpuReader::ThermGuardData>
        ThermGuardCpuReader::readCpuInfo()
    {
        HANDLE rawHandle = CreateFileW(
            L"\\\\.\\NyfariThermGuard",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        ScopedHandle handle(rawHandle);

        if (!handle.valid()) {
            ny::log::Logger::log(ny::log::Level::Warn,
                "ThermGuardCpuReader",
                "Falha ao abrir NyfariThermGuard. Error=" + std::to_string(GetLastError()));
            return std::nullopt;
        }

        ny::log::Logger::log(ny::log::Level::Debug,
            "ThermGuardCpuReader",
            "Device NyfariThermGuard aberto com sucesso");

        THERMGUARD_CPU_INFO info{};
        DWORD bytesReturned = 0;

        BOOL result = DeviceIoControl(
            handle.get(),
            IOCTL_THERMGUARD_QUERY_CPU_INFO,
            nullptr,
            0,
            &info,
            sizeof(info),
            &bytesReturned,
            nullptr
        );

        if (!result) {
            ny::log::Logger::log(ny::log::Level::Warn,
                "ThermGuardCpuReader",
                "IOCTL_THERMGUARD_QUERY_CPU_INFO falhou. Error=" + std::to_string(GetLastError()));
            return std::nullopt;
        }

        if (bytesReturned != sizeof(THERMGUARD_CPU_INFO)) {
            ny::log::Logger::log(ny::log::Level::Warn,
                "ThermGuardCpuReader",
                "Bytes retornados incorretos: " + std::to_string(bytesReturned));
            return std::nullopt;
        }

        ThermGuardData data;
        data.brandString = trimNullTerminated(info.BrandString, sizeof(info.BrandString));
        data.clockMHz = info.CurrentMHz;
        data.temperatureC = info.TemperatureC;
        data.powerMilliWatts = info.PowerMilliWatts;

        std::ostringstream oss;
        oss << "ThermGuard: Clock=" << data.clockMHz
            << "MHz Temp=" << data.temperatureC
            << "C Power=" << data.powerMilliWatts << "mW";

        ny::log::Logger::log(ny::log::Level::Info,
            "ThermGuardCpuReader",
            oss.str());

        return data;
    }

    std::optional<double>
        ThermGuardCpuReader::readTemperatureCelsius(int)
    {
        HANDLE rawHandle = CreateFileW(
            L"\\\\.\\NyfariThermGuard",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        ScopedHandle handle(rawHandle);

        if (!handle.valid()) {
            ny::log::Logger::log(ny::log::Level::Warn,
                "ThermGuardCpuReader",
                "Falha ao abrir driver para leitura de temperatura. Error=" + std::to_string(GetLastError()));
            return std::nullopt;
        }

        CPU_TEMPERATURE temp{};
        DWORD bytesReturned = 0;

        BOOL result = DeviceIoControl(
            handle.get(),
            IOCTL_THERMGUARD_QUERY_TEMPERATURE,
            nullptr,
            0,
            &temp,
            sizeof(temp),
            &bytesReturned,
            nullptr
        );

        if (!result || bytesReturned != sizeof(CPU_TEMPERATURE)) {
            ny::log::Logger::log(ny::log::Level::Warn,
                "ThermGuardCpuReader",
                "IOCTL_THERMGUARD_QUERY_TEMPERATURE falhou. Error=" + std::to_string(GetLastError()));
            return std::nullopt;
        }

        if (temp.IsValid == 0)
            return std::nullopt;

        ny::log::Logger::log(ny::log::Level::Debug,
            "ThermGuardCpuReader",
            "Temperatura lida via ThermGuard: " + std::to_string(temp.TemperatureCelsius) + "C");

        return static_cast<double>(temp.TemperatureCelsius);
    }

}
