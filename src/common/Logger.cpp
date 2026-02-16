// src/common/log/Logger.cpp
#include "../include/ny/log/Logger.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <windows.h>

namespace ny::log {

    static std::mutex g_mutex;
    static std::string g_logFilePath;
    static std::ofstream g_ofs;

    void Logger::init(std::string logFilePath) {
        std::lock_guard<std::mutex> lk(g_mutex);
        g_logFilePath = std::move(logFilePath);
        if (!g_logFilePath.empty()) {
            g_ofs.open(g_logFilePath, std::ios::app);
        }
    }

    void Logger::shutdown() {
        std::lock_guard<std::mutex> lk(g_mutex);
        if (g_ofs.is_open()) {
            g_ofs.flush();
            g_ofs.close();
        }
        g_logFilePath.clear();
    }

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

    void Logger::log(Level level, const std::string& tag, const std::string& msg) {
        std::lock_guard<std::mutex> lk(g_mutex);
        std::ostringstream ss;
        ss << nowIso() << " [" << levelToString(level) << "] " << tag << " - " << msg << "\n";
        const std::string out = ss.str();

        // OutputDebugString (visible no DebugView / Visual Studio Output)
        OutputDebugStringA(out.c_str());

        // Arquivo (se aberto)
        if (g_ofs.is_open()) {
            g_ofs << out;
            g_ofs.flush();
        }
    }

} // namespace ny::log
