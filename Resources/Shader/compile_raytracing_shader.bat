@ECHO ON
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 ../Resources/Shader/raytrace.rgen -o ../Resources/Shader/raytrace.rgen.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 ../Resources/Shader/raytrace.rchit -o ../Resources/Shader/raytrace.rchit.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 ../Resources/Shader/raytrace.rmiss -o ../Resources/Shader/raytrace.rmiss.spv
C:/VulkanSDK/1.2.176.1/Bin/glslc.exe --target-env=vulkan1.2 ../Resources/Shader/raytraceShadow.rmiss -o ../Resources/Shader/raytraceShadow.rmiss.spv