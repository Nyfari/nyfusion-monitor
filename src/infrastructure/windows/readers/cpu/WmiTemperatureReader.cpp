// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
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
#include <windows.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

namespace ny::infra::windows::reader {

    std::optional<double>
    WmiTemperatureReader::readTemperatureCelsius() const {

        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr))
            return std::nullopt;

        IWbemLocator* locator = nullptr;
        hr = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID*)&locator
        );

        if (FAILED(hr))
            return std::nullopt;

        IWbemServices* services = nullptr;
        hr = locator->ConnectServer(
            _bstr_t(L"ROOT\\WMI"),
            nullptr,nullptr,0,0,0,0,&services
        );

        if (FAILED(hr))
            return std::nullopt;

        IEnumWbemClassObject* enumerator = nullptr;
        hr = services->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
            WBEM_FLAG_FORWARD_ONLY,
            nullptr,
            &enumerator
        );

        if (FAILED(hr))
            return std::nullopt;

        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;

        if (enumerator->Next(WBEM_INFINITE,1,&obj,&ret)==S_OK) {
            VARIANT vt;
            obj->Get(L"CurrentTemperature",0,&vt,0,0);
            double c = (vt.uintVal / 10.0) - 273.15;
            VariantClear(&vt);
            obj->Release();
            return c;
        }

        return std::nullopt;
    }
}
