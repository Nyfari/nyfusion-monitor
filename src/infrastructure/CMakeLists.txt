add_library(ny_infrastructure STATIC
        linux/factory/LinuxProviderFactory.cpp
        linux/factory/LinuxProviderFactory.hpp

        linux/providers/LinuxCPUProvider.cpp
        linux/providers/LinuxCPUProvider.hpp
        linux/providers/LinuxGPUProvider.cpp
        linux/providers/LinuxGPUProvider.hpp
        linux/providers/LinuxMemoryProvider.cpp
        linux/providers/LinuxMemoryProvider.hpp

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
