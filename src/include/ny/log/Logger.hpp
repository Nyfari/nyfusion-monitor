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
#pragma once
#include <string>

namespace ny::log {

    enum class Level { Debug, Info, Warn, Error };

    class Logger {
    public:
        // Inicializa o logger. Se logFilePath vazio -> só OutputDebugString.
        static void init(const std::string& logFilePath);
        static void shutdown();

        static void log(Level level, const std::string& tag, const std::string& msg);

    private:
        static std::string levelToString(Level l);
    };

} // namespace ny::log
