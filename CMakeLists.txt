cmake_minimum_required(VERSION 3.20)
project(RayTracingGPUVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")

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
        ${CMAKE_CURRENT_BINARY_DIR}/include/shader_path.hpp
)

target_link_libraries(RayTracingGPUVulkan glfw Vulkan::Vulkan)

function(compile_glsl stage glsl_file spv_file)
add_custom_command(COMMENT "Compiling ${stage} shader"
                    OUTPUT ${spv_file}
                    COMMAND Vulkan::glslangValidator -V --target-env vulkan1.3 -S ${stage} -o ${spv_file}
                            ${glsl_file}
                    MAIN_DEPENDENCY ${glsl_file}
                    DEPENDS ${glsl_file} Vulkan::glslangValidator)
endfunction()
function(compile_glsl_help stage)
	compile_glsl(${stage}
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.${stage}
		${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.${stage}.spv
	)
    set(
        ${stage}_shader_path
        "${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.${stage}.spv"
        PARENT_SCOPE
    )
    target_sources(RayTracingGPUVulkan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.${stage} ${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.${stage}.spv)
endfunction()

compile_glsl_help(rgen)
compile_glsl_help(rint)
compile_glsl_help(rchit)
compile_glsl_help(rmiss)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader_path.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/include/shader_path.hpp
)

target_include_directories(RayTracingGPUVulkan PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
