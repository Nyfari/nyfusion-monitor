cmake_minimum_required(VERSION 3.22)

project(ny_fusion_monitor
        VERSION 0.1.12
        LANGUAGES CXX
)

# =========================
# C++ Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================
# Output directories
# =========================
# Base directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Per configuration (Debug / Release / etc)
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${CONFIG})

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${CONFIG})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER}
            ${CMAKE_SOURCE_DIR}/bin/${CONFIG})
endforeach()

# =========================
# Global compile options
# =========================
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# =========================
# Subprojects
# =========================
add_subdirectory(src)
