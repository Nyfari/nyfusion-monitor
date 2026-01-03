add_library(ny_domain INTERFACE)

target_include_directories(ny_domain
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_domain
        INTERFACE
        ny_core
)
