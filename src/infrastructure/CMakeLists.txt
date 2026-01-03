add_library(ny_infrastructure INTERFACE)

target_include_directories(ny_infrastructure
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_infrastructure
        INTERFACE
        ny_domain
)
