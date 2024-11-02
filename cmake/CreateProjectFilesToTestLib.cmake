# how to exclude files from static code analysis
# https://stackoverflow.com/questions/49591804/clang-tidy-cmake-exclude-file-from-check/49591908#49591908
# here we create a library with all specific render agnostic code so we can test it without static code analysis
add_library(
  PROJECT_FILES_TO_TEST OBJECT
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
  ${APP_FILTER}
  ${UTIL_FILTER}
  ${VULKAN_BASE_FILTER}
  ${SHADER_HOST_DEVICE_FILTER})

target_include_directories(
  PROJECT_FILES_TO_TEST
  PUBLIC include/renderer
         include/renderer/pushConstants
         include/renderer/accelerationStructures
         include/vulkan_base
         include/gui
         include/common
         include/scene
         include/window
         include/memory
         include/util
         include/app
         Resources/Shaders/hostDevice
         include/)

target_include_directories(PROJECT_FILES_TO_TEST PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  PROJECT_FILES_TO_TEST
  PRIVATE ${CMAKE_DL_LIBS}
          Threads::Threads
          Vulkan::Vulkan
          glfw
          imgui
          stb
          glm
          tinyobjloader
          vma
          ktx)

target_link_libraries(PROJECT_FILES_TO_TEST PRIVATE GSL)

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(PROJECT_FILES_TO_TEST INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(PROJECT_FILES_TO_TEST INTERFACE -w)
else()
  target_compile_options(PROJECT_FILES_TO_TEST INTERFACE -w)
endif()

set_target_properties(PROJECT_FILES_TO_TEST PROPERTIES CXX_CLANG_TIDY "" CXX_CPPCHECK "")
