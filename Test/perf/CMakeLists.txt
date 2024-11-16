set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/cmake/filters/SetShaderFilters.cmake)
include(${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/cmake/filters/SetProjectFilters.cmake)
include(${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetTestFilters.cmake)

set(PERF_TEST_SUITE perfTestSuite)

add_executable(
  ${PERF_TEST_SUITE}
  # all test specific files -- start
  ${RENDERER_PERF_TEST_FILTER}
  # all test specific files -- end
  ${RASTER_SHADER_FILTER}
  ${RAYTRACING_SHADER_FILTER}
  ${COMMON_SHADER_FILTER}
  ${POST_SHADER_FILTER}
  ${BRDF_SHADER_FILTER}
  ${PBR_SHADER_FILTER}
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
string(
  REPLACE ";"
          " "
          ShaderIncludesString
          "${ShaderIncludes}")

message("The shader include string is the following: ${ShaderIncludesString}")

if(NOT MSVC)
  target_compile_definitions(
    ${PERF_TEST_SUITE}
    PRIVATE RELATIVE_RESOURCE_PATH="/../Resources/"
            RELATIVE_INCLUDE_PATH="/../Src/GraphicsEngineVulkan/"
            RELATIVE_IMGUI_FONTS_PATH="/../ExternalLib/IMGUI/misc/fonts/"
            ShaderIncludesString="${ShaderIncludesString}")
else()
  target_compile_definitions(
    ${PERF_TEST_SUITE}
    PRIVATE RELATIVE_RESOURCE_PATH="/../../Resources/"
            RELATIVE_INCLUDE_PATH="/../../Src/GraphicsEngineVulkan/"
            RELATIVE_IMGUI_FONTS_PATH="/../../ExternalLib/IMGUI/misc/fonts/"
            ShaderIncludesString="${ShaderIncludesString}")
endif()

configure_file(${PROJECT_SRC_DIR}/VulkanRendererConfig.hpp.in "${PROJECT_SRC_DIR}/renderer/VulkanRendererConfig.hpp")

target_include_directories(
  ${PERF_TEST_SUITE}
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

target_include_directories(${PERF_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${PERF_TEST_SUITE}
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
         myproject_options
         myproject_warnings
  PRIVATE benchmark::benchmark
          benchmark::benchmark_main
          GSL
          spdlog::spdlog)

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${PERF_TEST_SUITE} INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${PERF_TEST_SUITE} INTERFACE -w)
else()
  target_compile_options(${PERF_TEST_SUITE} INTERFACE -w)
endif()
