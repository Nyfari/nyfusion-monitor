add_library(ny_infrastructure STATIC)

# ============================================================
# COMMON (compilado em todas as plataformas)
# ============================================================

target_sources(ny_infrastructure PRIVATE
        common/ISensor.hpp
        common/cpu/ICpuFrequencySensor.hpp
        common/cpu/ICpuPowerSensor.hpp
        common/cpu/ICpuTemperatureSensor.hpp
        common/cpu/ICpuUsageSensor.hpp
        common/IMemorySensor.hpp

        stub/factory/StubProviderFactory.cpp
        stub/factory/StubProviderFactory.hpp

        stub/providers/StubCPUProvider.cpp
        stub/providers/StubCPUProvider.hpp
        stub/providers/StubGPUProvider.cpp
        stub/providers/StubGPUProvider.hpp
        stub/providers/StubMemoryProvider.cpp
        stub/providers/StubMemoryProvider.hpp
)

# ============================================================
# LINUX
# ============================================================

if(UNIX AND NOT APPLE)

    target_sources(ny_infrastructure PRIVATE
            linux/factory/LinuxProviderFactory.cpp
            linux/factory/LinuxProviderFactory.hpp

            linux/providers/LinuxCPUProvider.cpp
            linux/providers/LinuxCPUProvider.hpp
            linux/providers/LinuxGPUProvider.cpp
            linux/providers/LinuxGPUProvider.hpp
            linux/providers/LinuxMemoryProvider.cpp
            linux/providers/LinuxMemoryProvider.hpp

            linux/sensors/cpu/LinuxCpuFrequencySensor.cpp
            linux/sensors/cpu/LinuxCpuFrequencySensor.hpp
            linux/sensors/cpu/LinuxCpuTemperatureSensor.cpp
            linux/sensors/cpu/LinuxCpuTemperatureSensor.hpp
            linux/sensors/cpu/LinuxCpuUsageSensor.cpp
            linux/sensors/cpu/LinuxCpuUsageSensor.hpp
            linux/sensors/ram/LinuxMemorySensor.cpp
            linux/sensors/ram/LinuxMemorySensor.hpp

            linux/readers/ProcStatReader.cpp
            linux/readers/ProcStatReader.hpp
    )

endif()

# ============================================================
# WINDOWS
# ============================================================

if(WIN32)

    target_sources(ny_infrastructure PRIVATE
            windows/factory/WindowsProviderFactory.cpp
            windows/factory/WindowsProviderFactory.hpp

            windows/providers/WindowsCPUProvider.cpp
            windows/providers/WindowsCPUProvider.hpp
            windows/providers/WindowsMemoryProvider.cpp
            windows/providers/WindowsMemoryProvider.hpp

            windows/sensors/cpu/WindowsCpuUsageSensor.cpp
            windows/sensors/cpu/WindowsCpuUsageSensor.hpp
            windows/sensors/cpu/WindowsCpuFrequencySensor.cpp
            windows/sensors/cpu/WindowsCpuFrequencySensor.hpp
            windows/sensors/cpu/WindowsCpuTemperatureSensor.cpp
            windows/sensors/cpu/WindowsCpuTemperatureSensor.hpp

            windows/readers/cpu/SystemTimesReader.cpp
            windows/readers/cpu/SystemTimesReader.hpp
            windows/readers/cpu/PdhCpuReader.cpp
            windows/readers/cpu/PdhCpuReader.hpp
            windows/readers/cpu/ProcessorPowerReader.cpp
            windows/readers/cpu/ProcessorPowerReader.hpp
            windows/readers/cpu/RegistryCpuReader.cpp
            windows/readers/cpu/RegistryCpuReader.hpp
            windows/readers/cpu/LogicalProcessorReader.cpp
            windows/readers/cpu/LogicalProcessorReader.hpp
            windows/readers/cpu/WmiTemperatureReader.cpp
            windows/readers/cpu/WmiTemperatureReader.hpp
    )

    target_link_libraries(ny_infrastructure
            PUBLIC
            pdh
            powrprof
            wbemuuid
    )

endif()

# ============================================================
# MACOS (PREPARADO PARA FUTURO)
# ============================================================

if(APPLE)

    target_sources(ny_infrastructure PRIVATE
            mac/factory/MacProviderFactory.cpp
            mac/factory/MacProviderFactory.hpp

            mac/providers/MacCPUProvider.cpp
            mac/providers/MacCPUProvider.hpp
            mac/providers/MacMemoryProvider.cpp
            mac/providers/MacMemoryProvider.hpp

            mac/sensors/cpu/MacCpuUsageSensor.cpp
            mac/sensors/cpu/MacCpuUsageSensor.hpp

            mac/readers/MacSystemReader.cpp
            mac/readers/MacSystemReader.hpp
    )

endif()

# ============================================================
# INCLUDE DIRECTORIES
# ============================================================

target_include_directories(ny_infrastructure
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============================================================
# LINK COM DOMAIN
# ============================================================

target_link_libraries(ny_infrastructure
        PUBLIC
        ny_domain
)