# update current positions
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(EXTERNAL_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../ExternalLib/)
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Shaders/)

include(cmake/filters/SetShaderFilters.cmake)
include(cmake/filters/SetProjectFilters.cmake)
include(cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetSourceGroups.cmake)

set(OpenGlEnigneTargetName "OpenGLEngine")

# add the executable
add_executable(
  ${OpenGlEnigneTargetName}
  ${PBR_SHADER_FILTER}
  ${BRDF_SHADER_FILTER}
  ${CAMERA_FILTER}
  ${COMMON_SHADER_FILTER}
  ${CLOUDS_SHADER_FILTER}
  ${LOADING_SHADER_FILTER}
  ${RASTERIZER_SHADER_FILTER}
  ${RASTERIZER_SHADOWS_SHADER_FILTER}
  ${SKYBOX_SHADER_FILTER}
  ${GUI_FILTER}
  # this is great; no CPPCHECK,CLANG_TIDY here
  $<TARGET_OBJECTS:IMGUI>
  $<TARGET_OBJECTS:GLAD>
  ${WINDOW_FILTER}
  ${DEBUG_FILTER}
  ${UTIL_FILTER}
  ${COMPUTE_FILTER}
  ${RENDERER_FILTER}
  ${DEFERRED_FILTER}
  ${LOAD_SCREEN_FILTER}
  ${ATMOSPHERE_EFFECTS_FILTER}
  ${LIGHT_FILTER}
  ${D_LIGHT_FILTER}
  ${P_LIGHT_FILTER}
  ${CLOUD_FILTER}
  ${SHADOWS_FILTER}
  ${SKY_BOX_FILTER}
  ${TEXTURE_FILTER}
  ${SCENE_FILTER}
  ${COMMON_FILTER}
  ${APP_FILTER})

target_include_directories(
  ${OpenGlEnigneTargetName}
  PUBLIC ${PROJECT_INCLUDE_DIR}
         ${PROJECT_INCLUDE_DIR}camera
         ${PROJECT_INCLUDE_DIR}compute
         ${PROJECT_INCLUDE_DIR}renderer
         ${PROJECT_INCLUDE_DIR}renderer/loading_screen
         ${PROJECT_INCLUDE_DIR}renderer/deferred
         ${PROJECT_INCLUDE_DIR}debug
         ${PROJECT_INCLUDE_DIR}window
         ${PROJECT_INCLUDE_DIR}scene
         ${PROJECT_INCLUDE_DIR}scene/atmospheric_effects/clouds
         ${PROJECT_INCLUDE_DIR}scene/light
         ${PROJECT_INCLUDE_DIR}scene/light/directional_light
         ${PROJECT_INCLUDE_DIR}scene/light/point_light
         ${PROJECT_INCLUDE_DIR}scene/shadows
         ${PROJECT_INCLUDE_DIR}scene/sky_box
         ${PROJECT_INCLUDE_DIR}scene/texture
         ${PROJECT_INCLUDE_DIR}gui
         ${PROJECT_INCLUDE_DIR}util
         ${SHADER_SRC_DIR}hostDevice)

target_include_directories(${OpenGlEnigneTargetName} PRIVATE ${OPENGL_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${OpenGlEnigneTargetName}
  PUBLIC # for imgui
         ${CMAKE_DL_LIBS}
         Threads::Threads
         ${OPENGL_LIBRARIES}
         Vulkan::Vulkan
         glfw
         imgui
         stb
         glm
         tinyobjloader
         glad
         # enable compiler warnings
         myproject_warnings
         # enable sanitizers
         myproject_options
  PRIVATE GSL spdlog::spdlog nlohmann_json::nlohmann_json)
