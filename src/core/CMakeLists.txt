add_library(ny_core INTERFACE
        common/Types.hpp
)

target_include_directories(ny_core
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)