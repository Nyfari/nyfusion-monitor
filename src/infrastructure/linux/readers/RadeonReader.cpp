// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: RadeonReader.hpp
 * Source File Name: RadeonReader.cpp
 * Created by
 * @author Marcos Henrique
 * @date 01/03/2026
 */
#include "RadeonReader.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <cstdio>
#include <memory>

namespace ny::infra::linux::reader {

    std::string RadeonReader::readFile(const std::string& path) noexcept {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }
        std::string content;
        std::getline(file, content);

        // Remove whitespace final
        while (!content.empty() && std::isspace(content.back())) {
            content.pop_back();
        }
        return content;
    }

    uint64_t RadeonReader::readUint64(const std::string& path) noexcept {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return 0;
            }
            uint64_t value = 0;
            file >> value;
            return value;
        } catch (...) {
            return 0;
        }
    }

    uint32_t RadeonReader::readUint32(const std::string& path) noexcept {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return 0;
            }
            uint32_t value = 0;
            file >> value;
            return value;
        } catch (...) {
            return 0;
        }
    }

    float RadeonReader::readFloat(const std::string& path) noexcept {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return 0.0f;
            }
            float value = 0.0f;
            file >> value;
            return value;
        } catch (...) {
            return 0.0f;
        }
    }

    std::string RadeonReader::getGpuSysPath() {
        // Procura primeira GPU AMD em /sys/class/drm/
        std::string drmPath = "/sys/class/drm";

        if (!std::filesystem::exists(drmPath)) {
            return "";
        }

        for (const auto& entry : std::filesystem::directory_iterator(drmPath)) {
            if (!entry.is_directory()) continue;

            std::string name = entry.path().filename().string();
            if (!name.starts_with("card")) continue;

            std::string devicePath = entry.path().string() + "/device";
            std::string vendorFile = devicePath + "/vendor";

            std::string vendor = readFile(vendorFile);
            // AMD = 0x1002
            if (vendor.find("0x1002") != std::string::npos) {
                return devicePath;
            }
        }

        return "";
    }

    std::string RadeonReader::getHwmonPath(const std::string& /* gpuPath */) {
        // Procura hwmon correspondente em /sys/class/hwmon/
        std::string hwmonPath = "/sys/class/hwmon";
        if (!std::filesystem::exists(hwmonPath)) {
            return "";
        }

        for (const auto& entry : std::filesystem::directory_iterator(hwmonPath)) {
            if (!entry.is_directory()) continue;

            std::string name = entry.path().filename().string();
            if (!name.starts_with("hwmon")) continue;

            // Verifica se é a hwmon da nossa GPU (amdgpu)
            std::string namePath = entry.path().string() + "/name";
            std::string hwmonName = readFile(namePath);

            if (hwmonName.find("amdgpu") != std::string::npos) {
                return entry.path().string();
            }
        }

        return "";
    }

    /**
     * @brief Executa comando e captura output
     */
    static std::string executeCommand(const char* cmd) noexcept {
        std::string result;
        FILE* pipe = popen(cmd, "r");
        if (!pipe) return result;

        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        // Remove newline final
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
            result.pop_back();
        }
        return result;
    }

    /**
     * @brief Extrai nome limpo da GPU do lspci
     */
    static std::string cleanupGpuName(const std::string& lspciLine) noexcept {
        // Exemplo: "0a:00.0 VGA compatible controller: AMD/ATI] Navi 22 [Radeon RX 6700/6700 XT/6750 XT..."
        // Queremos: "Radeon RX 6700/6700 XT/6750 XT / 6800M/6850M XT"

        std::string result;

        // Procura por "[Radeon" e extrai dali
        size_t radeonPos = lspciLine.find("[Radeon");
        if (radeonPos != std::string::npos) {
            result = lspciLine.substr(radeonPos + 1);  // Remove o "[", fica "Radeon..."
        } else if (lspciLine.find("Radeon") != std::string::npos) {
            // Fallback: se não tiver "[", procura só "Radeon"
            size_t pos = lspciLine.find("Radeon");
            result = lspciLine.substr(pos);
        } else {
            // Fallback final: pega tudo depois de "AMD/ATI"
            size_t pos = lspciLine.find("AMD/ATI");
            if (pos == std::string::npos) {
                pos = lspciLine.find("AMD");
            }
            if (pos != std::string::npos) {
                result = lspciLine.substr(pos);
                // Remove "AMD/ATI]" ou "AMD]" do início
                while (!result.empty() && (result[0] == ']' || result[0] == ' ' || result[0] == 'A' ||
                                           result[0] == 'M' || result[0] == 'D' || result[0] == '/')) {
                    result.erase(0, 1);
                }
            }
        }

        // Remove " (rev XX)" ou similar no final
        size_t revPos = result.find(" (rev");
        if (revPos != std::string::npos) {
            result = result.substr(0, revPos);
        }

        // Remove "]" no final se existir
        if (!result.empty() && result.back() == ']') {
            result.pop_back();
        }

        // Remove trailing whitespace
        while (!result.empty() && std::isspace(result.back())) {
            result.pop_back();
        }

        return result;
    }

    /**
     * @brief Limpa versão de Mesa removendo suffixes desnecessários
     */
    static std::string cleanMesaVersion(const std::string& version) noexcept {
        std::string clean = version;

        // Remove "-ubuntu..." ou outros suffixes
        size_t suffixPos = clean.find('-');
        if (suffixPos != std::string::npos) {
            clean = clean.substr(0, suffixPos);
        }

        // Remove "OpenGL" se estiver no final
        size_t openglPos = clean.find("OpenGL");
        if (openglPos != std::string::npos) {
            clean = clean.substr(0, openglPos);
        }

        // Remove whitespace final
        while (!clean.empty() && std::isspace(clean.back())) {
            clean.pop_back();
        }

        return clean;
    }

    /**
     * @brief Detecta drivers AMD: Kernel (AMDGPU), API (Mesa), e Compute (ROCm)
     */
    static std::string detectAmdDriver() noexcept {
        std::string kernelVersion;
        std::string mesaVersion;
        std::string rocmVersion;

        // Tenta obter versão do AMDGPU kernel driver
        std::string result = executeCommand("modinfo amdgpu 2>/dev/null | grep -i '^version' | head -1");
        if (!result.empty()) {
            size_t pos = result.find(':');
            if (pos != std::string::npos) {
                kernelVersion = result.substr(pos + 1);
                // Remove whitespace
                while (!kernelVersion.empty() && std::isspace(kernelVersion.front())) {
                    kernelVersion.erase(0, 1);
                }
            }
        }

        // Tenta obter versão do Mesa (OpenGL/Vulkan)
        // Procura especificamente por "Mesa x.y.z"
        result = executeCommand("glxinfo 2>/dev/null | grep -i 'mesa' | grep -i 'version\\|release' | head -3");
        if (!result.empty()) {
            // Procura por "Mesa" seguido de versão
            size_t mesaPos = result.find("Mesa");
            if (mesaPos != std::string::npos) {
                // Tenta encontrar versão depois de "Mesa"
                size_t pos = mesaPos + 4;  // Pula "Mesa"

                // Pula espaços
                while (pos < result.length() && std::isspace(result[pos])) {
                    pos++;
                }

                // Extrai números e pontos
                if (pos < result.length() && std::isdigit(result[pos])) {
                    size_t start = pos;
                    while (pos < result.length() && (std::isdigit(result[pos]) || result[pos] == '.')) {
                        pos++;
                    }
                    mesaVersion = result.substr(start, pos - start);
                }
            }
        }

        // Se glxinfo não funcionou, tenta vulkaninfo
        if (mesaVersion.empty()) {
            result = executeCommand("vulkaninfo 2>/dev/null | grep -i 'mesa' | head -1");
            if (!result.empty()) {
                size_t mesaPos = result.find("Mesa");
                if (mesaPos != std::string::npos) {
                    size_t pos = mesaPos + 4;
                    while (pos < result.length() && std::isspace(result[pos])) {
                        pos++;
                    }
                    if (pos < result.length() && std::isdigit(result[pos])) {
                        size_t start = pos;
                        while (pos < result.length() && (std::isdigit(result[pos]) || result[pos] == '.')) {
                            pos++;
                        }
                        mesaVersion = result.substr(start, pos - start);
                    }
                }
            }
        }

        // Tenta obter versão do ROCm (Compute API)
        // Primeiro tenta hipcc (HIP compiler)
        result = executeCommand("hipcc --version 2>/dev/null | head -1");
        if (!result.empty()) {
            // Procura por padrão de versão "x.y.z"
            size_t pos = 0;
            while (pos < result.length()) {
                if (std::isdigit(result[pos])) {
                    size_t start = pos;
                    int dotCount = 0;
                    // Extrai até 2 pontos (x.y.z)
                    while (pos < result.length() && (std::isdigit(result[pos]) || result[pos] == '.')) {
                        if (result[pos] == '.') dotCount++;
                        if (dotCount > 2) break;  // Para após terceira parte
                        pos++;
                    }
                    rocmVersion = result.substr(start, pos - start);
                    break;
                }
                pos++;
            }
        }

        // Se hipcc não funcionou, tenta rocm-smi
        if (rocmVersion.empty()) {
            result = executeCommand("rocm-smi --version 2>/dev/null");
            if (!result.empty()) {
                // Procura por padrão de versão "x.y.z"
                size_t pos = 0;
                while (pos < result.length()) {
                    if (std::isdigit(result[pos])) {
                        size_t start = pos;
                        int dotCount = 0;
                        // Extrai até 2 pontos (x.y.z)
                        while (pos < result.length() && (std::isdigit(result[pos]) || result[pos] == '.')) {
                            if (result[pos] == '.') dotCount++;
                            if (dotCount > 2) break;  // Para após terceira parte
                            pos++;
                        }
                        rocmVersion = result.substr(start, pos - start);
                        if (!rocmVersion.empty()) break;
                    }
                    pos++;
                }
            }
        }

        // Monta string final com nomenclatura melhorada
        std::string result_str;

        // Adiciona Kernel Driver
        if (!kernelVersion.empty()) {
            result_str += "Kernel (" + kernelVersion + ")";
        } else {
            // Verifica se amdgpu está carregado, mesmo sem versão
            result = executeCommand("lsmod 2>/dev/null | grep -i amdgpu");
            if (!result.empty()) {
                result_str += "Kernel (version unknown)";
            }
        }

        // Adiciona API Driver (Mesa)
        if (!mesaVersion.empty()) {
            if (!result_str.empty()) result_str += " | ";
            result_str += "Mesa (" + mesaVersion + ")";
        } else {
            // Verifica se Mesa está disponível mesmo sem versão clara
            result = executeCommand("glxinfo 2>/dev/null | grep -i 'mesa'");
            if (!result.empty()) {
                if (!result_str.empty()) result_str += " | ";
                result_str += "Mesa (version unknown)";
            }
        }

        // Adiciona Compute (ROCm) se disponível
        if (!rocmVersion.empty()) {
            if (!result_str.empty()) result_str += " | ";
            result_str += "ROCm (" + rocmVersion + ")";
        } else {
            // Verifica se ROCm está instalado mesmo sem versão
            result = executeCommand("rocminfo 2>/dev/null | grep -i 'runtime version'");
            if (!result.empty()) {
                // Tenta extrair versão de rocminfo
                size_t pos = result.find_last_of("0123456789");
                if (pos != std::string::npos) {
                    size_t start = pos;
                    while (start > 0 && (std::isdigit(result[start-1]) || result[start-1] == '.')) {
                        start--;
                    }
                    std::string ver = result.substr(start, pos - start + 1);
                    if (!ver.empty()) {
                        if (!result_str.empty()) result_str += " | ";
                        result_str += "ROCm (" + ver + ")";
                    }
                }
            }
        }

        // Se nada foi encontrado, tenta detectar radeon antigo
        if (result_str.empty()) {
            result = executeCommand("lsmod 2>/dev/null | grep -i radeon");
            if (!result.empty()) {
                result_str = "Radeon (legacy)";
            } else {
                result_str = "Unknown";
            }
        }

        return result_str;
    }

    RadeonSample RadeonReader::readGpu() const {
        RadeonSample sample;

        std::string gpuPath = getGpuSysPath();
        if (gpuPath.empty()) {
            return sample;
        }

        std::string hwmonPath = getHwmonPath(gpuPath);

        // Vendor
        sample.vendor = "AMD";

        // Model: extrai de lspci com limpeza
        std::string lspciOutput = executeCommand("lspci | grep -i 'vga\\|3d'");
        if (!lspciOutput.empty()) {
            sample.model = cleanupGpuName(lspciOutput);
        }

        if (sample.model.empty()) {
            sample.model = "RADEON (Unknown)";
        }

        // VRAM Total
        sample.vramBytes = readUint64(gpuPath + "/mem_info_vram_total");

        // VRAM em uso
        sample.vramUsedBytes = readUint64(gpuPath + "/mem_info_vram_used");

        // Hwmon data
        if (!hwmonPath.empty()) {
            // Utilização
            float utilFile = readFloat(hwmonPath + "/gpu_busy_percent");
            sample.utilizationPercent = utilFile;

            // Temperatura (em milidegrees, divide por 1000)
            float tempFile = readFloat(hwmonPath + "/temp1_input");
            sample.temperatureCelsius = tempFile / 1000.0f;

            // Frequência (em Hz, converte para MHz)
            uint32_t freqFile = readUint32(hwmonPath + "/freq1_input");
            sample.frequencyMHz = freqFile / 1000000;  // Hz → MHz

            // Potência (em microwatts, divide por 1000000 para Watts)
            float powerFile = readFloat(hwmonPath + "/power1_average");
            sample.powerWatts = powerFile / 1000000.0f;
        }

        // Driver: detecta Mesa ou AMDGPU
        sample.driverVersion = detectAmdDriver();

        return sample;
    }
}