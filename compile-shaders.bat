Pushd "%~dp0"

"%~dp0lib\VulkanSDK\1.2.182.0\Bin\glslc.exe" "%~dp0shaders\shader.comp" -o "cmake-build-debug\shader.comp.spv"
