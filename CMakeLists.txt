cmake_minimum_required(VERSION 3.20)
project(RayTracingGPUVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

include_directories(
        lib/glm
        lib/stb
        lib/Vulkan-Headers/Include
        lib/glfw/include
)

# https://www.glfw.org/download.html
# https://vulkan.lunarg.com/
link_directories(
        lib/glfw/lib-vc2019
        lib/Vulkan-Headers/Lib
)

add_executable(
        RayTracingGPUVulkan
        src/main.cpp
        src/vulkan_settings.h
        src/vulkan.h
        src/vulkan.cpp
        src/scene.h
        src/scene.cpp
        src/render_call_info.h
)

target_link_libraries(RayTracingGPUVulkan glfw3.lib vulkan-1.lib)
