include(GoogleTest)

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

set(COMPILE_TEST_SUITE compileTestSuite)

add_executable(
  ${COMPILE_TEST_SUITE}
  # all test specific files -- start
  ${RENDERER_COMPILE_TEST_FILTER}
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
  ${COMPILE_TEST_SUITE}
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

target_include_directories(${COMPILE_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${COMPILE_TEST_SUITE}
  PUBLIC gtest_main
         gtest
         ${CMAKE_DL_LIBS}
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

target_link_libraries(${COMPILE_TEST_SUITE} PRIVATE GSL)

if(NOT WINDOWS_CI)
  message(STATUS "WINDOWS_CI is OFF or not defined.")
  gtest_discover_tests(${COMPILE_TEST_SUITE} DISCOVERY_TIMEOUT 300)
else()
  message(STATUS "WINDOWS_CI is OFF or not defined.")
endif()

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMPILE_TEST_SUITE} INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMPILE_TEST_SUITE} INTERFACE -w)
else()
  target_compile_options(${COMPILE_TEST_SUITE} INTERFACE -w)
endif()

# make our sources available to testing framework
add_custom_target(
  resourcefilesForCompileTestSuite ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Test/Resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/Resources
          ${CMAKE_BINARY_DIR}/Test/Resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/ExternalLib/IMGUI/misc/fonts/
          ${CMAKE_BINARY_DIR}/Test/ExternalLib/IMGUI/misc/fonts/
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/include/renderer/
          ${CMAKE_BINARY_DIR}/Test/include/renderer
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/include/renderer/pushConstants
          ${CMAKE_BINARY_DIR}/Test/include/renderer/pushConstants
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/include/scene/
          ${CMAKE_BINARY_DIR}/Test/include/scene)

if(NOT WINDOWS_CI)
  message(STATUS "WINDOWS_CI is OFF or not defined.")
  add_dependencies(resourcefilesForCompileTestSuite ${PROJECT_NAME})

  add_dependencies(${COMPILE_TEST_SUITE} resourcefilesForCompileTestSuite ${PROJECT_NAME})
else()
  add_dependencies(resourcefilesForCompileTestSuite ${COMPILE_TEST_SUITE})
endif()
