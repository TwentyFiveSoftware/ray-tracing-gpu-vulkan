Pushd "%~dp0"

"%VULKAN_SDK%\Bin\glslc.exe" "%~dp0shaders\shader.rgen" -o "cmake-build-debug\shaders\shader.rgen.spv" --target-env=vulkan1.2
"%VULKAN_SDK%\Bin\glslc.exe" "%~dp0shaders\shader.rint" -o "cmake-build-debug\shaders\shader.rint.spv" --target-env=vulkan1.2
"%VULKAN_SDK%\Bin\glslc.exe" "%~dp0shaders\shader.rchit" -o "cmake-build-debug\shaders\shader.rchit.spv" --target-env=vulkan1.2
"%VULKAN_SDK%\Bin\glslc.exe" "%~dp0shaders\shader.rmiss" -o "cmake-build-debug\shaders\shader.rmiss.spv" --target-env=vulkan1.2
