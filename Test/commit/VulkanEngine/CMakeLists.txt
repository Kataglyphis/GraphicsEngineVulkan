include(GoogleTest)

# all testing related stuff
set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${PROJECT_SRC_DIR}cmake/filters/SetShaderFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetProjectFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetTestFilters.cmake)

set(COMMIT_TEST_SUITE commitTestSuite)

add_executable(
  ${COMMIT_TEST_SUITE}
  # all test specific files -- start
  ${RENDERER_COMMIT_TEST_FILTER}
  # all test specific files -- end
  ${RASTER_SHADER_FILTER}
  ${RAYTRACING_SHADER_FILTER}
  ${COMMON_SHADER_FILTER}
  ${POST_SHADER_FILTER}
  ${BRDF_SHADER_FILTER}
  ${PATH_TRACING_SHADER_FILTER}
  ${RENDERER_FILTER}
  ${PC_FILTER}
  ${AS_FILTER}
  ${SCENE_FILTER}
  ${WINDOW_FILTER}
  ${MEMORY_FILTER}
  ${COMMON_FILTER}
  ${GUI_FILTER}
  ${UTIL_FILTER}
  ${VULKAN_BASE_FILTER}
  ${SHADER_HOST_DEVICE_FILTER}
  $<TARGET_OBJECTS:IMGUI>)

  set(ShaderIncludes
  -I
  ${SHADER_SRC_DIR}common/
  -I
  ${SHADER_SRC_DIR}pbr/
  -I
  ${SHADER_SRC_DIR}pbr/brdf/
  -I
  ${SHADER_SRC_DIR}hostDevice/
  -I
  ${PROJECT_SRC_DIR}/renderer/
  -I
  ${PROJECT_SRC_DIR}/renderer/pushConstants/
  -I
  ${PROJECT_SRC_DIR}/scene/)
  
  # Join the paths into a single space-separated string
  string(REPLACE ";" " " ShaderIncludesString "${ShaderIncludes}")
  
  message("The shader include string is the following: ${ShaderIncludesString}")
  
  target_compile_definitions(${COMMIT_TEST_SUITE} PRIVATE
      RELATIVE_RESOURCE_PATH="/${WORKING_DIRECTORY}/Resources/"
      RELATIVE_INCLUDE_PATH="/${WORKING_DIRECTORY}/Src/GraphicsEngineVulkan/"
      RELATIVE_IMGUI_FONTS_PATH="/${WORKING_DIRECTORY}/ExternalLib/IMGUI/misc/fonts/"
      ShaderIncludesString="${ShaderIncludesString}"
  )

configure_file(${PROJECT_SRC_DIR}/VulkanRendererConfig.hpp.in "${PROJECT_SRC_DIR}/renderer/VulkanRendererConfig.hpp")

target_include_directories(
  ${COMMIT_TEST_SUITE}
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
         ${PROJECT_INCLUDE_DIR}
         ${SHADER_SRC_DIR}hostDevice)

target_include_directories(${COMMIT_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${COMMIT_TEST_SUITE}
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
  PRIVATE gtest gtest_main GSL spdlog::spdlog)

if(NOT WINDOWS_CI)
  message(STATUS "WINDOWS_CI is OFF or not defined.")
  gtest_discover_tests(${COMMIT_TEST_SUITE} DISCOVERY_TIMEOUT 300)
else()
  message(STATUS "WINDOWS_CI is OFF or not defined.")
endif()

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE -w)
else()
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE -w)
endif()
