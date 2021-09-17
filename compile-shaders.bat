Pushd "%~dp0"

"%~dp0lib\VulkanSDK\1.2.189.2\Bin\glslc.exe" "%~dp0shaders\shader.comp" -o "cmake-build-debug\shader.comp.spv"

"%~dp0lib\VulkanSDK\1.2.189.2\Bin\glslc.exe" "%~dp0shaders\shader.rgen" -o "cmake-build-debug\shader.rgen.spv" --target-env=vulkan1.2
"%~dp0lib\VulkanSDK\1.2.189.2\Bin\glslc.exe" "%~dp0shaders\shader.rint" -o "cmake-build-debug\shader.rint.spv" --target-env=vulkan1.2
"%~dp0lib\VulkanSDK\1.2.189.2\Bin\glslc.exe" "%~dp0shaders\shader.rchit" -o "cmake-build-debug\shader.rchit.spv" --target-env=vulkan1.2
"%~dp0lib\VulkanSDK\1.2.189.2\Bin\glslc.exe" "%~dp0shaders\shader.rmiss" -o "cmake-build-debug\shader.rmiss.spv" --target-env=vulkan1.2
