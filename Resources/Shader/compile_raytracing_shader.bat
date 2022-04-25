@ECHO ON
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 ../Resources/Shader/raytracing/raytrace.rgen -o ../Resources/Shader/raytracing\spv\raytrace.rgen.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 ../Resources/Shader/raytracing/raytrace.rchit -o ../Resources/Shader/raytracing\spv\raytrace.rchit.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 ../Resources/Shader/raytracing/raytrace.rmiss -o ../Resources/Shader/raytracing\spv\raytrace.rmiss.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 ../Resources/Shader/raytracing/shadow.rmiss -o ../Resources/Shader/raytracing\spv\shadow.rmiss.spv