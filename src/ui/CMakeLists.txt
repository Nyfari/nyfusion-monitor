find_package(Qt6 COMPONENTS Core Widgets Concurrent REQUIRED)

qt6_add_resources(ny_ui "resources.qrc")

add_library(ny_ui STATIC
        # Services & ViewModel (backend bridge)
        services/HardwareBackendService.cpp
        services/HardwareBackendService.hpp
        services/GameDetectionService.cpp
        services/GameDetectionService.hpp
        services/FrameMetricsService.cpp
        services/FrameMetricsService.hpp
        services/OverlayRuntimeController.cpp
        services/OverlayRuntimeController.hpp
        viewmodels/DashboardViewModel.cpp
        viewmodels/DashboardViewModel.hpp
        viewmodels/OverlaySettingsViewModel.cpp
        viewmodels/OverlaySettingsViewModel.hpp

        # Custom widgets
        widgets/CircularGauge.cpp
        widgets/CircularGauge.hpp
        widgets/LineAreaChartWidget.cpp
        widgets/LineAreaChartWidget.hpp
        widgets/SparklineWidget.cpp
        widgets/SparklineWidget.hpp
        widgets/LineChartWidget.cpp
        widgets/LineChartWidget.hpp
        widgets/LoadBarWidget.cpp
        widgets/LoadBarWidget.hpp

        # Dashboard panels
        dashboard/GpuPanel.cpp
        dashboard/GpuPanel.hpp
        dashboard/CpuPanel.cpp
        dashboard/CpuPanel.hpp
        dashboard/ChartAreaPanel.cpp
        dashboard/ChartAreaPanel.hpp
        dashboard/DashboardPage.cpp
        dashboard/DashboardPage.hpp

        # Main window
        qt/OverlaySettingsDialog.cpp
        qt/OverlaySettingsDialog.hpp
        qt/OverlayWindow.cpp
        qt/OverlayWindow.hpp
        qt/MainWindow.cpp
        qt/MainWindow.hpp
)

set_target_properties(ny_ui PROPERTIES
        AUTOMOC ON
)

target_include_directories(ny_ui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src/hook   # ny_fps_shared.h
)

target_link_libraries(ny_ui
        PUBLIC
        ny_application
        ny_infrastructure
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
)

if (WIN32)
    target_link_libraries(ny_ui PUBLIC Psapi)
endif()

# ── XCB Present: contagem real de FPS via eventos de apresentação de frames ──
if (UNIX AND NOT APPLE)
    find_library(XCB_LIB NAMES xcb HINTS /usr/lib/x86_64-linux-gnu /usr/lib)
    if (XCB_LIB)
        target_link_libraries(ny_ui PUBLIC ${XCB_LIB})
    endif ()

    # O .so de link (sem versão) pode não existir se libxcb-present-dev não está
    # instalado; tenta o .so versionado diretamente como fallback.
    find_library(XCB_PRESENT_LIB NAMES xcb-present
            HINTS /usr/lib/x86_64-linux-gnu /usr/lib)
    if (NOT XCB_PRESENT_LIB)
        foreach (SO_SUFFIX ".so.0" ".so.0.0.0")
            if (EXISTS "/usr/lib/x86_64-linux-gnu/libxcb-present${SO_SUFFIX}")
                set(XCB_PRESENT_LIB "/usr/lib/x86_64-linux-gnu/libxcb-present${SO_SUFFIX}")
                break()
            endif ()
        endforeach ()
    endif ()
    if (XCB_PRESENT_LIB)
        target_link_libraries(ny_ui PUBLIC ${XCB_PRESENT_LIB})
        target_compile_definitions(ny_ui PUBLIC NY_HAS_XCB_PRESENT)
        message(STATUS "[ny_ui] XCB Present encontrado: ${XCB_PRESENT_LIB}")
    else ()
        message(WARNING "[ny_ui] libxcb-present não encontrado – FPS via XCB Present desativado")
    endif ()
endif ()
