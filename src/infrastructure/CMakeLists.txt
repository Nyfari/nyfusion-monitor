add_library(ny_infrastructure STATIC
        common/ISensor.hpp
        common/cpu/ICpuFrequencySensor.hpp
        common/cpu/ICpuPowerSensor.hpp
        common/cpu/ICpuTemperatureSensor.hpp
        common/cpu/ICpuUsageSensor.hpp
        common/IMemorySensor.hpp

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

        stub/factory/StubProviderFactory.cpp
        stub/factory/StubProviderFactory.hpp

        stub/providers/StubCPUProvider.cpp
        stub/providers/StubCPUProvider.hpp
        stub/providers/StubGPUProvider.cpp
        stub/providers/StubGPUProvider.hpp
        stub/providers/StubMemoryProvider.cpp
        stub/providers/StubMemoryProvider.hpp
)

target_include_directories(ny_infrastructure
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_infrastructure
        PUBLIC
        ny_domain
)
