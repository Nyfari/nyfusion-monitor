// KernelCpuTempReader_diag.cpp (substitua temporariamente)
#include "KernelCpuTempReader.hpp"
#include <windows.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include "../include/shared.h"

namespace ny::infra::windows::reader {

    static void hex_dump(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i += 16) {
            std::cout << std::setw(4) << std::setfill('0') << std::hex << i << ": ";
            for (size_t j = 0; j < 16 && i + j < len; ++j)
                std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)data[i + j] << ' ';
            std::cout << std::dec << '\n';
        }
    }

    std::optional<double> KernelCpuTempReader::readTemperatureCelsius(int coreIndex) {
        constexpr wchar_t DEVICE_NAME[] = L"\\\\.\\NyCpuTemp";

        HANDLE h = CreateFileW(DEVICE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            std::cerr << "[KernelCpuTempReader] CreateFileW failed. GetLastError() = " << err << "\n";
            // Erros comuns: 2 (ERROR_FILE_NOT_FOUND) = driver/symbolic link não existe
            // 5 (ERROR_ACCESS_DENIED) = sem permissão
            return std::nullopt;
        }

        CPU_TEMP_REQUEST req{};
        req.coreIndex = static_cast<int32_t>(coreIndex);

        const DWORD outSize = static_cast<DWORD>(sizeof(CPU_TEMP_RESULT) + (8 - 1) * sizeof(CPU_TEMP_ENTRY)); // espaço para 8 entries
        std::vector<uint8_t> outBuf(outSize);
        DWORD bytesReturned = 0;

        BOOL ok = DeviceIoControl(h,
            IOCTL_READ_CPU_TEMPERATURE,
            &req, sizeof(req),
            outBuf.data(), outSize,
            &bytesReturned, nullptr);
        DWORD dioErr = ok ? 0 : GetLastError();
        std::cout << "[KernelCpuTempReader] DeviceIoControl returned ok=" << ok << " dioErr=" << dioErr
            << " bytesReturned=" << bytesReturned << "\n";

        if (!ok) {
            CloseHandle(h);
            return std::nullopt;
        }

        if (bytesReturned < sizeof(uint32_t)) {
            std::cerr << "[KernelCpuTempReader] bytesReturned too small\n";
            CloseHandle(h);
            return std::nullopt;
        }

        // Dump do buffer para diagnóstico
        hex_dump(outBuf.data(), bytesReturned);

        CPU_TEMP_RESULT* res = reinterpret_cast<CPU_TEMP_RESULT*>(outBuf.data());
        if (res->entryCount == 0) {
            std::cerr << "[KernelCpuTempReader] entryCount == 0\n";
            CloseHandle(h);
            return std::nullopt;
        }

        const CPU_TEMP_ENTRY& e = res->entries[0];
        std::cout << "[KernelCpuTempReader] entry[0]: coreIndex=" << e.coreIndex
            << " success=" << (int)e.success << " temp=" << e.temperatureC << "\n";

        CloseHandle(h);
        if (!e.success) return std::nullopt;
        return e.temperatureC;
    }

} // namespace ny::infra::windows::reader
