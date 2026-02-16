// include/ny/log/Logger.hpp
#pragma once
#include <string>
#include <mutex>

namespace ny::log {
    enum class Level { Debug, Info, Warn, Error };
    class Logger {
    public:
        static void init(std::string logFilePath);
        static void shutdown();
        static void log(Level level, const std::string& tag, const std::string& msg);
    private:
        static std::string levelToString(Level l);
    };
} // namespace ny::log
