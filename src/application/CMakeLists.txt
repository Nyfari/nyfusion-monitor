add_library(ny_application INTERFACE)

target_include_directories(ny_application
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_application
        INTERFACE
        ny_domain
)
