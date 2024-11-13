// the configured options and settings for renderer
// https://riptutorial.com/cmake/example/32603/using-cmake-to-define-the-version-number-for-cplusplus-usage
#ifndef RENDERER_CONFIG_GUARD
#define RENDERER_CONFIG_GUARD

#define VulkanRenderer_VERSION_MAJOR "1"
#define VulkanRenderer_VERSION_MINOR "3"

#define VULKAN_VERSION_MAJOR "1"
#define VULKAN_VERSION_MINOR "3"

#define GLSLC_EXE "/bin/glslc"
// change this path when install
#define RELATIVE_RESOURCE_PATH "/../Resources/" // /..
#define RELATIVE_INCLUDE_PATH "/../include/" // /..
#define RELATIVE_IMGUI_FONTS_PATH "/../ExternalLib/IMGUI/misc/fonts/" // /..

#endif
