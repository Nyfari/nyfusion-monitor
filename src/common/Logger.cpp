// NyFusion Monitor
// Copyright (C) 2026 Nyfari
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * NyFusion Monitor
 * Copyright (C) 2026 Nyfari
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Header File Name: Logger.hpp
 * Source File Name: Logger.cpp
 * Created by
 * @author Marcos Henrique
 * @date 16/02/2026
 */
#include "../include/ny/log/Logger.hpp"
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <windows.h>

namespace ny::log {

    static std::mutex g_mutex;
    static std::ofstream g_ofs;
    static std::string g_path;

    static std::string nowIso() {
        using namespace std::chrono;
        auto t = system_clock::now();
        auto tt = system_clock::to_time_t(t);
        auto ms = duration_cast<milliseconds>(t.time_since_epoch()) % 1000;
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setw(3) << std::setfill('0') << ms.count();
        return ss.str();
    }

    std::string Logger::levelToString(Level l) {
        switch (l) {
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO";
        case Level::Warn:  return "WARN";
        case Level::Error: return "ERROR";
        default: return "UNK";
        }
    }

    void Logger::init(const std::string& logFilePath) {
        std::lock_guard<std::mutex> lk(g_mutex);
        if (!g_ofs.is_open() && !logFilePath.empty()) {
            g_path = logFilePath;
            try {
                std::filesystem::create_directories(std::filesystem::path(g_path).parent_path());
                g_ofs.open(g_path, std::ios::app);
            }
            catch (...) {
                g_path.clear();
            }
        }
    }

    void Logger::shutdown() {
        std::lock_guard<std::mutex> lk(g_mutex);
        if (g_ofs.is_open()) {
            g_ofs.flush();
            g_ofs.close();
        }
        g_path.clear();
    }

    void Logger::log(Level level, const std::string& tag, const std::string& msg) {
        std::lock_guard<std::mutex> lk(g_mutex);
        std::ostringstream ss;
        ss << nowIso() << " [" << levelToString(level) << "] " << tag << " - " << msg << "\n";
        const std::string out = ss.str();

        // OutputDebugString (DebugView / Visual Studio)
        OutputDebugStringA(out.c_str());

        // File
        if (g_ofs.is_open()) {
            g_ofs << out;
            g_ofs.flush();
        }
    }

} // namespace ny::log
