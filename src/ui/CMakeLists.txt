add_library(ny_ui STATIC
        qt/MainWindow.cpp
        qt/MainWindow.hpp
)

target_include_directories(ny_ui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_ui
        PRIVATE
        ny_application
)
