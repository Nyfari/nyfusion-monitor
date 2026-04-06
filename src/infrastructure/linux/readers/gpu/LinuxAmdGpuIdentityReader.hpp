// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
/**
 * @file LinuxAmdGpuIdentityReader.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 *
 * Resolução do nome da GPU AMD — cadeia de prioridade:
 *  1. /usr/share/libdrm/amdgpu.ids  (device_id + revision_id → nome exato)
 *  2. pci.ids subsystem lookup       (sv + sd → nome do board AIB)
 *  3. pci.ids device lookup          (device_id → nome genérico da família)
 *  4. Fallback técnico limpo         "AMD Radeon [0xXXXX]"
 */

#include <optional>
#include <string>
#include <fstream>
#include <array>
#include <filesystem>
#include <cctype>
#include <vector>

#include "LinuxAmdGpuSysfs.hpp"

namespace ny::infra::linux::reader::gpu {

    struct LinuxAmdGpuIdentityRaw final {
        std::string vendor;
        std::string model;
    };

    class LinuxAmdGpuIdentityReader final {
    public:
        [[nodiscard]] std::optional<LinuxAmdGpuIdentityRaw> read() const {
            const auto cardPath = sysfs::resolveAmdCardPath();
            if (!cardPath.has_value()) return std::nullopt;

            const auto vendorId   = sysfs::readText(cardPath.value() / "device/vendor");
            const auto deviceId   = sysfs::readText(cardPath.value() / "device/device");
            const auto revisionId = sysfs::readText(cardPath.value() / "device/revision");
            const auto subsysVend = sysfs::readText(cardPath.value() / "device/subsystem_vendor");
            const auto subsysDev  = sysfs::readText(cardPath.value() / "device/subsystem_device");

            LinuxAmdGpuIdentityRaw raw{};
            raw.vendor = "AMD";

            const auto vendorHex = normalizeHexId(vendorId.value_or(""));
            const auto deviceHex = normalizeHexId(deviceId.value_or(""));
            const auto revHex    = normalizeRevId(revisionId.value_or(""));
            const auto svHex     = normalizeHexId(subsysVend.value_or(""));
            const auto sdHex     = normalizeHexId(subsysDev.value_or(""));

            // 1. amdgpu.ids — fonte mais precisa: device_id + revision_id → nome exato
            //    Ex: 73DF, C0 → "AMD Radeon RX 6750 XT"
            if (!deviceHex.empty() && !revHex.empty()) {
                const auto amdName = lookupAmdgpuIds(deviceHex, revHex);
                if (amdName.has_value() && !amdName->empty()) {
                    raw.model = *amdName;
                    return raw;
                }
            }

            // 2. pci.ids subsystem — nome do board AIB (ex: "Radeon RX 6750 XT PULSE OC")
            if (!svHex.empty() && !sdHex.empty()) {
                const auto subsysName = lookupPciSubsystem(vendorHex, deviceHex, svHex, sdHex);
                if (subsysName.has_value() && !subsysName->empty()) {
                    raw.model = formatDirectName(*subsysName);
                    return raw;
                }
            }

            // 3. pci.ids device — nome genérico da família com extração de variante
            const auto pciName = lookupPciDevice(vendorHex, deviceHex);
            if (pciName.has_value() && !pciName->empty()) {
                raw.model = formatCommercialName(*pciName);
                return raw;
            }

            // 4. Fallback técnico limpo
            raw.model = deviceHex.empty()
                ? "AMD GPU"
                : "AMD Radeon [0x" + deviceHex + "]";
            return raw;
        }

    private:
        // ── Utilitários ──────────────────────────────────────────────────────

        static std::string normalizeHexId(const std::string& raw) {
            std::string out;
            out.reserve(raw.size());
            for (char ch : raw) {
                if (std::isxdigit(static_cast<unsigned char>(ch)))
                    out.push_back(static_cast<char>(
                        std::tolower(static_cast<unsigned char>(ch))));
            }
            while (!out.empty() && out.front() == '0' && out.size() > 4)
                out.erase(out.begin());
            if (out.size() > 4)
                out = out.substr(out.size() - 4);
            return out;
        }

        // Normaliza revision ID para 2 dígitos hex lowercase (ex: "0xc0" → "c0")
        static std::string normalizeRevId(const std::string& raw) {
            std::string out;
            for (char ch : raw) {
                if (std::isxdigit(static_cast<unsigned char>(ch)))
                    out.push_back(static_cast<char>(
                        std::tolower(static_cast<unsigned char>(ch))));
            }
            while (out.size() > 2 && out.front() == '0')
                out.erase(out.begin());
            if (out.size() == 1)
                out = "0" + out;
            return out;
        }

        static std::string trim(const std::string& v) {
            std::size_t s = 0;
            while (s < v.size() &&
                   std::isspace(static_cast<unsigned char>(v[s])) != 0) ++s;
            std::size_t e = v.size();
            while (e > s &&
                   std::isspace(static_cast<unsigned char>(v[e - 1])) != 0) --e;
            return v.substr(s, e - s);
        }

        static bool hexIdMatch(const std::string& line, const std::string& hexId) {
            if (line.size() < hexId.size()) return false;
            for (std::size_t i = 0; i < hexId.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(line[i])) !=
                    std::tolower(static_cast<unsigned char>(hexId[i])))
                    return false;
            }
            if (line.size() == hexId.size()) return true;
            return std::isspace(static_cast<unsigned char>(line[hexId.size()])) != 0;
        }

        static bool hexPairMatch(const std::string& line,
                                  const std::string& hexA,
                                  const std::string& hexB) {
            const std::size_t minLen = hexA.size() + 1 + hexB.size();
            if (line.size() < minLen) return false;
            for (std::size_t i = 0; i < hexA.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(line[i])) !=
                    std::tolower(static_cast<unsigned char>(hexA[i])))
                    return false;
            }
            if (line[hexA.size()] != ' ') return false;
            for (std::size_t i = 0; i < hexB.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(line[hexA.size() + 1 + i])) !=
                    std::tolower(static_cast<unsigned char>(hexB[i])))
                    return false;
            }
            const std::size_t afterPair = hexA.size() + 1 + hexB.size();
            if (afterPair >= line.size()) return true;
            return std::isspace(static_cast<unsigned char>(line[afterPair])) != 0;
        }

        // ── 1. amdgpu.ids ─────────────────────────────────────────────────────
        // Formato: device_id,   revision_id,    product_name
        // Ex:      73DF,        C0,             AMD Radeon RX 6750 XT
        static std::optional<std::string> lookupAmdgpuIds(
            const std::string& deviceHex, const std::string& revHex)
        {
            constexpr const char* kPath = "/usr/share/libdrm/amdgpu.ids";
            std::ifstream file(kPath);
            if (!file.is_open()) return std::nullopt;

            const std::string revNorm = normalizeRevId(revHex);

            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;

                const auto c1 = line.find(',');
                if (c1 == std::string::npos) continue;
                const auto c2 = line.find(',', c1 + 1);
                if (c2 == std::string::npos) continue;

                const auto devField  = trim(line.substr(0, c1));
                const auto revField  = trim(line.substr(c1 + 1, c2 - c1 - 1));
                const auto nameField = trim(line.substr(c2 + 1));

                if (devField.size() != 4 || nameField.empty()) continue;

                // Compara device_id (4 hex) case-insensitive
                bool devMatch = true;
                for (std::size_t i = 0; i < 4 && devMatch; ++i) {
                    if (std::tolower(static_cast<unsigned char>(devField[i])) !=
                        std::tolower(static_cast<unsigned char>(
                            i < deviceHex.size() ? deviceHex[i] : '\0')))
                        devMatch = false;
                }
                if (!devMatch) continue;

                // Compara revision_id (2 hex) case-insensitive
                if (normalizeRevId(revField) != revNorm) continue;

                return nameField;
            }
            return std::nullopt;
        }

        // ── 2. pci.ids subsystem ──────────────────────────────────────────────
        static std::optional<std::string> lookupPciSubsystem(
            const std::string& vendorHex, const std::string& deviceHex,
            const std::string& svHex, const std::string& sdHex)
        {
            if (svHex.empty() || sdHex.empty()) return std::nullopt;
            constexpr std::array<const char*, 2> kFiles = {
                "/usr/share/misc/pci.ids", "/usr/share/hwdata/pci.ids"
            };
            for (const auto* f : kFiles) {
                auto r = lookupSubsystemInFile(f, vendorHex, deviceHex, svHex, sdHex);
                if (r.has_value()) return r;
            }
            return std::nullopt;
        }

        static std::optional<std::string> lookupSubsystemInFile(
            const std::filesystem::path& filePath,
            const std::string& vendorHex, const std::string& deviceHex,
            const std::string& svHex, const std::string& sdHex)
        {
            std::ifstream file(filePath);
            if (!file.is_open()) return std::nullopt;

            std::string line;
            bool inVendor = false;
            bool inDevice = false;

            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                if (line[0] != '\t') {
                    inVendor = hexIdMatch(line, vendorHex);
                    inDevice = false;
                    continue;
                }
                if (!inVendor) continue;
                if (line.size() > 1 && line[1] == '\t') {
                    if (!inDevice) continue;
                    const auto rest = trim(line.substr(2));
                    if (hexPairMatch(rest, svHex, sdHex)) {
                        const std::size_t afterPair = svHex.size() + 1 + sdHex.size();
                        const auto split = rest.find_first_of(" \t", afterPair);
                        if (split != std::string::npos) {
                            auto name = trim(rest.substr(split));
                            if (!name.empty()) return name;
                        }
                    }
                    continue;
                }
                inDevice = hexIdMatch(line.substr(1), deviceHex);
            }
            return std::nullopt;
        }

        // ── 3. pci.ids device ─────────────────────────────────────────────────
        static std::optional<std::string> lookupPciDevice(
            const std::string& vendorHex, const std::string& deviceHex)
        {
            if (vendorHex.empty() || deviceHex.empty()) return std::nullopt;
            constexpr std::array<const char*, 2> kFiles = {
                "/usr/share/misc/pci.ids", "/usr/share/hwdata/pci.ids"
            };
            for (const auto* f : kFiles) {
                auto r = lookupDeviceInFile(f, vendorHex, deviceHex);
                if (r.has_value()) return r;
            }
            return std::nullopt;
        }

        static std::optional<std::string> lookupDeviceInFile(
            const std::filesystem::path& filePath,
            const std::string& vendorHex, const std::string& deviceHex)
        {
            std::ifstream file(filePath);
            if (!file.is_open()) return std::nullopt;

            std::string line;
            bool inVendor = false;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                if (line[0] != '\t') {
                    inVendor = hexIdMatch(line, vendorHex);
                    continue;
                }
                if (!inVendor) continue;
                if (line.size() > 1 && line[1] == '\t') continue;
                const auto trimmed = line.substr(1);
                if (hexIdMatch(trimmed, deviceHex)) {
                    const auto split = trimmed.find_first_of(" \t");
                    if (split == std::string::npos) return std::nullopt;
                    auto name = trim(trimmed.substr(split + 1));
                    return name.empty()
                        ? std::nullopt
                        : std::optional<std::string>{std::move(name)};
                }
            }
            return std::nullopt;
        }

        // ── Formatação ────────────────────────────────────────────────────────

        static std::string formatDirectName(const std::string& rawName) {
            std::string s = trim(rawName);
            if (!s.empty() && s.front() == '[' && s.back() == ']')
                s = trim(s.substr(1, s.size() - 2));
            if (s.empty()) return "AMD GPU";
            if (s.rfind("AMD ", 0) == 0) return s;
            if (s.rfind("Radeon", 0) == 0) return "AMD " + s;
            return s;
        }

        static std::string formatCommercialName(const std::string& rawName) {
            std::string s = trim(rawName);
            const auto ob = s.find('[');
            if (ob != std::string::npos) {
                const auto cb = s.rfind(']');
                if (cb != std::string::npos && cb > ob) {
                    const auto inside = trim(s.substr(ob + 1, cb - ob - 1));
                    if (!inside.empty()) {
                        s = (inside.find('/') != std::string::npos)
                            ? pickBestVariant(inside)
                            : inside;
                    }
                }
            }
            s = trim(s);
            if (s.empty()) return "AMD GPU";
            if (s.rfind("AMD ", 0) == 0) return s;
            if (s.rfind("Radeon", 0) == 0) return "AMD " + s;
            return "AMD " + s;
        }

        static std::string pickBestVariant(const std::string& slashList) {
            std::vector<std::string> tokens;
            std::size_t pos = 0;
            while (true) {
                const auto slash = slashList.find('/', pos);
                if (slash == std::string::npos) {
                    tokens.push_back(trim(slashList.substr(pos)));
                    break;
                }
                tokens.push_back(trim(slashList.substr(pos, slash - pos)));
                pos = slash + 1;
            }
            if (tokens.empty()) return slashList;

            std::string prefix;
            const auto& first = tokens[0];
            for (std::size_t i = 0; i + 1 < first.size(); ++i) {
                if (first[i] == ' ' &&
                    std::isdigit(static_cast<unsigned char>(first[i + 1])))
                    prefix = first.substr(0, i + 1);
            }

            for (const auto& token : tokens) {
                if (token.empty()) continue;
                std::string full = token;
                if (token != tokens[0] && !prefix.empty() &&
                    std::isdigit(static_cast<unsigned char>(token.front())))
                    full = prefix + token;

                bool isMobile = false;
                for (std::size_t i = 1; i + 1 < full.size(); ++i) {
                    if (std::isdigit(static_cast<unsigned char>(full[i])) &&
                        (full[i + 1] == 'M' || full[i + 1] == 'm') &&
                        (i + 2 >= full.size() ||
                         !std::isalnum(static_cast<unsigned char>(full[i + 2])))) {
                        isMobile = true;
                        break;
                    }
                }
                if (!isMobile) return full;
            }
            return tokens.front();
        }
    };

} // namespace ny::infra::linux::reader::gpu
