cmake_minimum_required(VERSION 3.20)
project(RayTracingGPU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

include_directories(
        lib/glm
        lib/stb
)

link_directories(
        lib/glfw-3.3.4.bin.WIN64/lib-vc2019
        lib/VulkanSDK/1.2.182.0/Lib
)

add_executable(RayTracingGPUVulkan src/main.cpp)

target_link_libraries(RayTracingGPUVulkan glfw3.lib vulkan-1.lib)
