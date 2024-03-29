cmake_minimum_required(VERSION 3.20)
project(RayTracingGPUVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

find_package(Vulkan REQUIRED)

add_subdirectory(lib/glfw)

include_directories(
        lib/glm
        lib/stb
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

target_link_libraries(RayTracingGPUVulkan glfw Vulkan::Vulkan)
