add_subdirectory(core)
add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(ui)

add_executable(ny_fusion_monitor
        main.cpp
)

target_link_libraries(ny_fusion_monitor
        PRIVATE
        ny_application
        ny_ui
)

target_include_directories(ny_fusion_monitor
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)