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
#include "../include/ny/log/Logger.hpp"

#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <cmath>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace ny::infra::windows::reader {

    namespace {
        // RAII para inicialização COM (multithreaded). Não falha se outro componente já inicializou.
        class ComInit {
        public:
            ComInit() : ok(false) {
                HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
                    ok = SUCCEEDED(hr);
                }
                // Tenta inicializar segurança; não falha se já foi inicializado.
                HRESULT secHr = CoInitializeSecurity(
                    nullptr, -1, nullptr, nullptr,
                    RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr, EOAC_NONE, nullptr);
                (void)secHr;
            }
            ~ComInit() {
                CoUninitialize();
            }
            bool initialized() const noexcept { return ok; }
        private:
            bool ok;
        };

        // Converte VARIANT para double quando possível
        static std::optional<double> VariantToDouble(const VARIANT& vt) {
            if (vt.vt == VT_I4) return static_cast<double>(vt.lVal);
            if (vt.vt == VT_UI4) return static_cast<double>(vt.ulVal);
            if (vt.vt == VT_R4) return static_cast<double>(vt.fltVal);
            if (vt.vt == VT_R8) return static_cast<double>(vt.dblVal);
            if (vt.vt == VT_BSTR && vt.bstrVal != nullptr) {
                try {
                    _bstr_t b(vt.bstrVal);
                    std::wstring ws = static_cast<const wchar_t*>(b);
                    if (!ws.empty()) {
                        size_t idx = 0;
                        double d = std::stod(ws, &idx);
                        return d;
                    }
                }
                catch (...) {
                    return std::nullopt;
                }
            }
            return std::nullopt;
        }

        // Heurística para converter valores brutos WMI para Celsius
        static std::optional<double> ConvertRawToCelsius(double raw) {
            // Se raw muito grande, provavelmente décimos de Kelvin (ex: 3000 -> 300.0K)
            if (raw > 1000.0) {
                return (raw / 10.0) - 273.15;
            }
            // Kelvin inteiro
            if (raw > 200.0) {
                return raw - 273.15;
            }
            // Celsius plausível
            if (raw > -100.0 && raw < 200.0) {
                return raw;
            }
            // fallback: tentar décimos de Kelvin
            if (raw > 0.0) {
                return (raw / 10.0) - 273.15;
            }
            return std::nullopt;
        }
    } // namespace

    std::optional<double> WmiTemperatureReader::readTemperatureCelsius() const {
        using ny::log::Logger;
        Logger::log(ny::log::Level::Debug, "WmiTemperatureReader", "Starting WMI temperature read");

        ComInit com; // inicializa COM no escopo
        // Mesmo que CoInitializeEx retorne RPC_E_CHANGED_MODE, prosseguimos; ComInit cuida do CoUninitialize.

        IWbemLocator* locator = nullptr;
        IWbemServices* services = nullptr;
        IEnumWbemClassObject* enumerator = nullptr;
        IWbemClassObject* obj = nullptr;

        HRESULT hr = CoCreateInstance(
            CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&locator));
        if (FAILED(hr) || locator == nullptr) {
            Logger::log(ny::log::Level::Warn, "WmiTemperatureReader", "CoCreateInstance(WbemLocator) failed: " + std::to_string(hr));
            return std::nullopt;
        }

        hr = locator->ConnectServer(
            _bstr_t(L"ROOT\\WMI"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
        if (FAILED(hr) || services == nullptr) {
            Logger::log(ny::log::Level::Warn, "WmiTemperatureReader", "ConnectServer(ROOT\\WMI) failed: " + std::to_string(hr));
            locator->Release();
            return std::nullopt;
        }

        // Ajusta segurança do proxy para permitir chamadas
        hr = CoSetProxyBlanket(
            services,
            RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr, EOAC_NONE);
        if (FAILED(hr)) {
            Logger::log(ny::log::Level::Warn, "WmiTemperatureReader", "CoSetProxyBlanket failed: " + std::to_string(hr));
            services->Release();
            locator->Release();
            return std::nullopt;
        }

        // Query WMI
        const BSTR queryLang = SysAllocString(L"WQL");
        const BSTR query = SysAllocString(L"SELECT * FROM MSAcpi_ThermalZoneTemperature");
        hr = services->ExecQuery(
            queryLang,
            query,
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &enumerator);
        SysFreeString(queryLang);
        SysFreeString(query);

        if (FAILED(hr) || enumerator == nullptr) {
            Logger::log(ny::log::Level::Debug, "WmiTemperatureReader", "WMI query returned no enumerator or failed: " + std::to_string(hr));
            services->Release();
            locator->Release();
            return std::nullopt;
        }

        // Propriedades candidatas
        const std::vector<std::wstring> candidateProps = {
            L"CurrentTemperature",
            L"Temperature",
            L"CurrentReading"
        };

        double bestC = std::numeric_limits<double>::lowest();
        ULONG returned = 0;

        while (SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &obj, &returned)) && returned == 1) {
            // Tenta InstanceName para diagnóstico (se existir)
            VARIANT vtName; VariantInit(&vtName);
            if (SUCCEEDED(obj->Get(L"InstanceName", 0, &vtName, nullptr, nullptr)) && vtName.vt == VT_BSTR) {
                _bstr_t b(vtName.bstrVal);
                std::wstring instanceName = static_cast<const wchar_t*>(b);
                Logger::log(ny::log::Level::Debug, "WmiTemperatureReader", "Found instance: " + std::string(instanceName.begin(), instanceName.end()));
            }
            VariantClear(&vtName);

            for (const auto& prop : candidateProps) {
                VARIANT vtProp; VariantInit(&vtProp);
                HRESULT getHr = obj->Get(prop.c_str(), 0, &vtProp, nullptr, nullptr);
                if (SUCCEEDED(getHr) && vtProp.vt != VT_EMPTY && vtProp.vt != VT_NULL) {
                    auto rawOpt = VariantToDouble(vtProp);
                    if (rawOpt.has_value()) {
                        double raw = rawOpt.value();
                        auto cOpt = ConvertRawToCelsius(raw);
                        if (cOpt.has_value()) {
                            double celsius = cOpt.value();
                            Logger::log(ny::log::Level::Debug, "WmiTemperatureReader",
                                "Property " + std::string(prop.begin(), prop.end()) +
                                " raw=" + std::to_string(raw) +
                                " -> " + std::to_string(celsius) + " C");
                            if (celsius > bestC) bestC = celsius;
                        }
                        else {
                            Logger::log(ny::log::Level::Debug, "WmiTemperatureReader",
                                "Property " + std::string(prop.begin(), prop.end()) +
                                " raw=" + std::to_string(raw) + " could not be converted");
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
            Logger::log(ny::log::Level::Debug, "WmiTemperatureReader", "No valid temperature found via WMI");
            return std::nullopt;
        }

        Logger::log(ny::log::Level::Info, "WmiTemperatureReader", "WMI temperature read: " + std::to_string(bestC) + " C");
        return bestC;
    }

} // namespace ny::infra::windows::reader
