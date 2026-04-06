find_package(Qt6 COMPONENTS Core Widgets Concurrent REQUIRED)

qt6_add_resources(ny_ui "resources.qrc")

add_library(ny_ui STATIC
        # Services & ViewModel (backend bridge)
        services/HardwareBackendService.cpp
        services/HardwareBackendService.hpp
        viewmodels/DashboardViewModel.cpp
        viewmodels/DashboardViewModel.hpp

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
        qt/MainWindow.cpp
        qt/MainWindow.hpp
)

set_target_properties(ny_ui PROPERTIES
        AUTOMOC ON
)

target_include_directories(ny_ui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_ui
        PUBLIC
        ny_application
        ny_infrastructure
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
)
