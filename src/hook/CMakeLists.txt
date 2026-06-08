# ny_fps_hook — CMakeLists.txt
# Habilita linguagem C para compilar a biblioteca hook
enable_language(C)

add_library(ny_fps_hook SHARED ny_fps_hook.c)

set_target_properties(ny_fps_hook PROPERTIES
    C_STANDARD          11
    C_STANDARD_REQUIRED ON
    PREFIX              "lib"
    OUTPUT_NAME         "ny_fps_hook"
)

target_compile_options(ny_fps_hook PRIVATE
    -O2
    -fPIC
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_libraries(ny_fps_hook PRIVATE dl pthread)

add_custom_command(TARGET ny_fps_hook POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ny_fps_hook>
        "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/$<TARGET_FILE_NAME:ny_fps_hook>"
    COMMENT "Copiando libny_fps_hook.so para bin/$<CONFIG>/"
)
