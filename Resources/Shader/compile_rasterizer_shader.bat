@ECHO ON
C:/VulkanSDK/1.2.176.1/Bin32/glslc.exe -c ../Resources/Shader/shader.vert -I ../ExternalLib/GLM -I ../ExternalLib/GLM/detail 
C:/VulkanSDK/1.2.176.1/Bin32/glslc.exe -I ../ExternalLib/GLM -c ../Resources/Shader/shader.frag