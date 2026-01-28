add_library(ny_application STATIC
        services/HardwareMonitorService.cpp
        services/HardwareMonitorService.hpp
)

target_include_directories(ny_application
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_application
        PUBLIC
        ny_domain
)