// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: WmiTemperatureReader.hpp
 * Source File Name: WmiTemperatureReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 12/02/2026
 */
#include "WmiTemperatureReader.hpp"

 // Requer: Windows SDK, linkar com wbemuuid.lib, ole32.lib, oleaut32.lib
#include <optional>
#include <string>
#include <limits>
#include <vector>
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <cmath>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace ny::infra::windows::reader {

    class ComInit {
    public:
        ComInit() : initialized(false) {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
                initialized = SUCCEEDED(hr);
            }
            // Tentar CoInitializeSecurity, mas não falhar se já foi inicializado por outro componente.
            HRESULT secHr = CoInitializeSecurity(
                nullptr,
                -1,
                nullptr,
                nullptr,
                RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                nullptr,
                EOAC_NONE,
                nullptr);
            if (FAILED(secHr) && secHr != RPC_E_TOO_LATE && secHr != RPC_E_TOO_LATE /*explicit*/) {
                // Não abortamos aqui; deixamos o chamador decidir. (poderia logar)
            }
        }
        ~ComInit() {
            CoUninitialize();
        }
    private:
        bool initialized;
    };

    static std::optional<double> VariantToDouble(const VARIANT& vt) {
        if (vt.vt == VT_I4) {
            long v = vt.lVal;
            // Alguns sensores reportam em décimos de Kelvin (inteiro)
            return static_cast<double>(v);
        }
        if (vt.vt == VT_UI4) {
            unsigned long v = vt.ulVal;
            return static_cast<double>(v);
        }
        if (vt.vt == VT_R4) {
            return static_cast<double>(vt.fltVal);
        }
        if (vt.vt == VT_R8) {
            return vt.dblVal;
        }
        if (vt.vt == VT_BSTR) {
            // tentar converter BSTR para double
            _bstr_t b(vt.bstrVal);
            try {
                std::wstring ws = static_cast<const wchar_t*>(b);
                if (!ws.empty()) {
                    size_t idx = 0;
                    double d = std::stod(std::wstring(ws), &idx);
                    return d;
                }
            }
            catch (...) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    std::optional<double> WmiTemperatureReader::readTemperatureCelsius() const {
        ComInit com;

        IWbemLocator* locator = nullptr;
        IWbemServices* services = nullptr;
        IEnumWbemClassObject* enumerator = nullptr;
        IWbemClassObject* obj = nullptr;

        // Conectar ao WMI
        HRESULT hr = CoCreateInstance(
            CLSID_WbemLocator,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            reinterpret_cast<LPVOID*>(&locator));
        if (FAILED(hr) || locator == nullptr) {
            return std::nullopt;
        }

        hr = locator->ConnectServer(
            _bstr_t(L"ROOT\\WMI"),
            nullptr,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr,
            &services);
        if (FAILED(hr) || services == nullptr) {
            locator->Release();
            return std::nullopt;
        }

        // Ajustar segurança do proxy
        CoSetProxyBlanket(
            services,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE);

        // Query: MSAcpi_ThermalZoneTemperature é a mais comum para ACPI thermal zones
        hr = services->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &enumerator);

        if (FAILED(hr) || enumerator == nullptr) {
            services->Release();
            locator->Release();
            return std::nullopt;
        }

        double bestC = std::numeric_limits<double>::lowest();
        ULONG returned = 0;

        // Propriedades candidatas que alguns firmwares usam
        const std::vector<std::wstring> candidateProps = {
            L"CurrentTemperature",
            L"Temperature",
            L"CurrentReading"
        };

        // Iterar todos os objetos retornados
        while (SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &obj, &returned)) && returned == 1) {
            // Tentar cada propriedade candidata
            for (const auto& prop : candidateProps) {
                VARIANT vtProp;
                VariantInit(&vtProp);
                HRESULT getHr = obj->Get(prop.c_str(), 0, &vtProp, nullptr, nullptr);
                if (SUCCEEDED(getHr) && vtProp.vt != VT_EMPTY && vtProp.vt != VT_NULL) {
                    auto rawOpt = VariantToDouble(vtProp);
                    if (rawOpt.has_value()) {
                        double raw = rawOpt.value();
                        double celsius = std::numeric_limits<double>::quiet_NaN();

                        // Heurística de conversão:
                        // - Se valor parece ser em décimos de Kelvin (>= 1000), aplicar (v/10 - 273.15)
                        // - Se valor parece ser Kelvin inteiro (> 100), aplicar (v - 273.15)
                        // - Se valor parece já em Celsius (entre -100 e 200), usar diretamente
                        if (raw > 1000.0) {
                            // décimos de Kelvin
                            celsius = (raw / 10.0) - 273.15;
                        }
                        else if (raw > 200.0) {
                            // Kelvin inteiro
                            celsius = raw - 273.15;
                        }
                        else if (raw > -100.0 && raw < 200.0) {
                            // provavelmente Celsius
                            celsius = raw;
                        }
                        else {
                            // fallback: tentar interpretar como décimos de Kelvin
                            celsius = (raw / 10.0) - 273.15;
                        }

                        if (!std::isnan(celsius) && celsius > bestC) {
                            bestC = celsius;
                        }
                    }
                }
                VariantClear(&vtProp);
            }

            obj->Release();
            obj = nullptr;
        }

        enumerator->Release();
        services->Release();
        locator->Release();

        if (bestC == std::numeric_limits<double>::lowest()) {
            return std::nullopt;
        }
        return bestC;
    }

}