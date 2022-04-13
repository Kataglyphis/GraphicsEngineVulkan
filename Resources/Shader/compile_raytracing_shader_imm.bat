@ECHO ON
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 raytrace.rgen -o raytrace.rgen.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 raytrace.rchit -o raytrace.rchit.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 raytrace.rmiss -o raytrace.rmiss.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 raytraceShadow.rmiss -o raytraceShadow.rmiss.spv
PAUSE