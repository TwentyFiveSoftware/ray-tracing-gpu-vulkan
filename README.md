# Ray Tracing

<img src="https://github.com/TwentyFiveSoftware/ray-tracing-gpu-vulkan/blob/master/sceneRender.png">

## Overview

This is my take on [Peter Shirley's Ray Tracing in One Weekend](https://github.com/RayTracing/raytracing.github.io) book
series.

This project is using a
the [Vulkan Ray Tracing](https://www.khronos.org/blog/vulkan-ray-tracing-final-specification-release) extension to
render the scene. Using this extension over a compute shader approach has two main advantages:

- The extension provides a thing called ``acceleration structure`` which pre-optimizes the scene geometry for ray
  intersection tests.
- The extension provides a dedicated ``ray tracing pipeline`` which uses the
  dedicated [Ray Accelerators](https://www.amd.com/de/technologies/rdna-2) in the new AMD RDNA 2 GPUs or the
  dedicated [RT Cores](https://www.nvidia.com/en-us/design-visualization/technologies/turing-architecture/) in NVIDIA's
  RTX graphics cards. Using them speeds up the rendering process tremendously (see performance comparison below).

*For more in-depth details visit the official Vulkan ray tracing extension
guide [here](https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/extensions/ray_tracing.adoc).*

## Build & Run this project

1. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
2. Clone the repository
3. Initialize git submodules (dependencies)
	```sh
	git submodule update --init --recursive
	```
4. Build the project
   ```sh
   cmake -S . -B build
   cmake --build build --config Release
   ```
4. Run the executable
   ```sh
   ./build/Release/RayTracingGPUVulkan.exe
   ```

## My Ray Tracing series

This is the final part of my 3 project series. Before this project, I followed Peter Shirley' Ray Tracing series and
wrote a multi-threaded CPU ray tracer, as well as a GPU ray tracing implementation using a compute shader. The
performance differences are compared below.

- [CPU Ray Tracing](https://github.com/TwentyFiveSoftware/ray-tracing)
- [GPU Ray Tracing (Compute Shader)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu)
- [GPU Ray Tracing (Vulkan Ray Tracing extension)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu-vulkan)

## Performance

The performance was measured on the same scene (see image above) with the same amount of objects, the same recursive
depth, the same resolution (1920 x 1080). The measured times are averaged over multiple runs.

*Reference system: AMD Ryzen 9 5900X (12 Cores / 24 Threads) | AMD Radeon RX 6800 XT*

| | [CPU Ray Tracing](https://github.com/TwentyFiveSoftware/ray-tracing) | [GPU Ray Tracing (Compute Shader)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu) | [GPU Ray Tracing (Vulkan RT extension)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu-vulkan) |
| --- | --- | --- | --- |
| 1 sample / pixel | ~ 3,800 ms | 21.5 ms | 1.25 ms |
| 10,000 samples / pixel | ~ 10.5 h (extrapolated) | 215 s | 12.5 s |
