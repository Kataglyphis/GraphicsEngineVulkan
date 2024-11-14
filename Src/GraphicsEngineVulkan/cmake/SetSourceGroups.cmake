# hint for the IDE which belongs together
source_group("vulkan_base" FILES ${VULKAN_BASE_FILTER})
source_group("gui" FILES ${GUI_FILTER})
source_group("gui/imgui" FILES ${IMGUI_FILTER})
source_group("renderer" FILES ${RENDERER_FILTER})
source_group("renderer/pc" FILES ${PC_FILTER})
source_group("renderer/AS" FILES ${AS_FILTER})
source_group("scene" FILES ${SCENE_FILTER})
source_group("window" FILES ${WINDOW_FILTER})
source_group("memory" FILES ${MEMORY_FILTER})
source_group("common" FILES ${COMMON_FILTER})
source_group("app" FILES ${APP_FILTER})
source_group("util" FILES ${UTIL_FILTER})

source_group("shaders/hostDevice/" FILES ${SHADER_HOST_DEVICE_FILTER})
source_group("shaders/rasterizer/" FILES ${RASTER_SHADER_FILTER})
source_group("shaders/raytracing/" FILES ${RAYTRACING_SHADER_FILTER})
source_group("shaders/common/" FILES ${COMMON_SHADER_FILTER})
source_group("shaders/post/" FILES ${POST_SHADER_FILTER})
source_group("shaders/brdf/" FILES ${BRDF_SHADER_FILTER})
source_group("shaders/path_tracing/" FILES ${PATH_TRACING_SHADER_FILTER})