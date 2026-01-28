add_library(ny_domain INTERFACE
        hardware/CPUInfo.hpp
        hardware/GPUInfo.hpp
        hardware/MemoryInfo.hpp

        providers/CPUProvider.hpp
        providers/GPUProvider.hpp
        providers/MemoryProvider.hpp
)

target_include_directories(ny_domain
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ny_domain
        INTERFACE
        ny_core
)
