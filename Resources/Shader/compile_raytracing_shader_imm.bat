@ECHO ON
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 raytrace.rgen -o raytrace.rgen.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 raytrace.rchit -o raytrace.rchit.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 raytrace.rmiss -o raytrace.rmiss.spv
C:/VulkanSDK/1.3.204.1/Bin/glslc.exe --target-env=vulkan1.3 raytraceShadow.rmiss -o raytraceShadow.rmiss.spv
PAUSE