include(GoogleTest)

# all testing related stuff
set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineVulkan/)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}include/)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${WORKING_DIRECTORY}cmake/filters/SetShaderFilters.cmake)
include(${WORKING_DIRECTORY}cmake/filters/SetProjectFilters.cmake)
include(${WORKING_DIRECTORY}cmake/filters/SetExternalLibsFilters.cmake)

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
         gtest
         gtest_main
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
         myproject_options)

target_link_libraries(${COMMIT_TEST_SUITE} PRIVATE GSL)

if(NOT WINDOWS_CI)
  message(STATUS "WINDOWS_CI is OFF or not defined.")
  gtest_discover_tests(${COMMIT_TEST_SUITE} DISCOVERY_TIMEOUT 300)
else()
  message(STATUS "WINDOWS_CI is OFF or not defined.")
endif()

# good source: https://stackoverflow.com/questions/52255589/code-coverage-analysis-using-codecoverage-cmake-with-multiple-targets
option(COVERAGE "Enable Code Coverage" ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU|Clang")
  if(COVERAGE)
    include(cmake/CodeCoverage.cmake)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_xml(
      NAME
      coverage_xml
      EXECUTABLE
      ctest
      -j
      ${n_cores} # Executable in PROJECT_BINARY_DIR
      EXCLUDE
      "ExternalLib/*"
      "build/*"
      "Documents/*")
    setup_target_for_coverage_gcovr_html(
      NAME
      coverage_html
      EXECUTABLE
      ctest
      -j
      ${n_cores} # Executable in PROJECT_BINARY_DIR
      EXCLUDE
      "ExternalLib/*"
      "build/*"
      "Documents/*")

  endif()
endif()

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE -w)
else()
  target_compile_options(${COMMIT_TEST_SUITE} INTERFACE -w)
endif()
