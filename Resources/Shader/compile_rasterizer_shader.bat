@ECHO ON
C:\VulkanSDK\1.3.204.1\Bin\glslc.exe --target-env=vulkan1.3 ..\Resources\Shader\rasterizer\shader.vert -o ..\Resources\Shader\rasterizer\spv\shader.vert.spv 
C:\VulkanSDK\1.3.204.1\Bin\glslc.exe --target-env=vulkan1.3 ..\Resources\Shader\rasterizer\shader.frag -o ..\Resources\Shader\rasterizer\spv\shader.frag.spv