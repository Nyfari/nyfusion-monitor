add_subdirectory(core)
add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(ui)

# Hook de LD_PRELOAD — libny_fps_hook.so (conta frames de jogos)
if (UNIX AND NOT APPLE)
    add_subdirectory(hook)
endif()

add_executable(ny_fusion_monitor
        main.cpp
  )

target_link_libraries(ny_fusion_monitor
        PRIVATE
        ny_application
        ny_infrastructure
        ny_ui
)

target_include_directories(ny_fusion_monitor
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)