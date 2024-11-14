# update current positions
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(EXTERNAL_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../ExternalLib/)
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Shaders/)

include(cmake/filters/SetShaderFilters.cmake)
include(cmake/filters/SetProjectFilters.cmake)
include(cmake/SetSourceGroups.cmake)
include(cmake/filters/SetExternalLibsFilters.cmake)

file(GLOB_RECURSE VULKANRENDERER_SOURCES
    "*.hpp"
    "*.cpp")

# add the executable
add_executable(
  ${PROJECT_NAME}
  ${RASTER_SHADER_FILTER}
  ${RAYTRACING_SHADER_FILTER}
  ${COMMON_SHADER_FILTER}
  ${POST_SHADER_FILTER}
  ${BRDF_SHADER_FILTER}
  ${PBR_SHADER_FILTER}
  ${PATH_TRACING_SHADER_FILTER}
  ${VULKANRENDERER_SOURCES}
  # this is great; no CPPCHECK,CLANG_TIDY here
  $<TARGET_OBJECTS:IMGUI>)

# # Recursively gather all include directories under PROJECT_INCLUDE_DIR
# file(GLOB_RECURSE PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIR}/*)

# # Set include directories recursively for the target
# target_include_directories(
#   ${PROJECT_NAME}
#   PUBLIC ${PROJECT_INCLUDE_DIR} ${PROJECT_INCLUDE_DIRS}
# )

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${PROJECT_INCLUDE_DIR}renderer
         ${PROJECT_INCLUDE_DIR}renderer/pushConstants
         ${PROJECT_INCLUDE_DIR}renderer/accelerationStructures
         ${PROJECT_INCLUDE_DIR}vulkan_base
         ${PROJECT_INCLUDE_DIR}gui
         ${PROJECT_INCLUDE_DIR}common
         ${PROJECT_INCLUDE_DIR}scene
         ${PROJECT_INCLUDE_DIR}window
         ${PROJECT_INCLUDE_DIR}memory
         ${PROJECT_INCLUDE_DIR}util
         ${PROJECT_INCLUDE_DIR}app
         ${SHADER_SRC_DIR}hostDevice
         ${PROJECT_INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_DL_LIBS}
         Threads::Threads
         Vulkan::Vulkan
         glfw
         imgui
         stb
         glm
         tinyobjloader
         vma
         ktx
         # enable compiler warnings
         myproject_warnings
         # enable sanitizers
         myproject_options
  PRIVATE GSL spdlog::spdlog nlohmann_json::nlohmann_json)
