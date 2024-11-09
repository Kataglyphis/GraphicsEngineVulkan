cmake_minimum_required(VERSION 3.28.3)

# set the project name
project(
  GraphicsEngine
  VERSION 1.3
  DESCRIPTION "Epic Graphics"
  HOMEPAGE_URL "https://jonasheinle.de/"
  LANGUAGES CXX C)

set(AUTHOR "Jotrockenmitlocken")

# Set WINDOWS_CI to OFF by default
# WINDOWS CI is only able to verify successful compiling ... Windows ... lel
# i will stay on linux ...
set(WINDOWS_CI
    OFF
    CACHE BOOL "Enable Windows CI build options")

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/ProjectOptions.cmake)

myproject_setup_options()

myproject_global_options()

myproject_local_options()

include(cmake/SystemLibDependencies.cmake)
include(Src/GraphicsEngineVulkan/cmake/filters/SetExternalLibsFilters.cmake)
add_subdirectory(ExternalLib)

add_subdirectory(Src)

# compile shaders as part of my cmake routine right away :)
include(cmake/CompileShadersToSPV.cmake)

foreach(Shader ${POST_SHADER_FILTER})
  add_shader(${PROJECT_NAME} ${Shader})
endforeach()

foreach(Shader ${RAYTRACING_SHADER_FILTER})
  add_shader(${PROJECT_NAME} ${Shader})
endforeach()

foreach(Shader ${RASTER_SHADER_FILTER})
  add_shader(${PROJECT_NAME} ${Shader})
endforeach()

foreach(Shader ${PATH_TRACING_SHADER_FILTER})
  add_shader(${PROJECT_NAME} ${Shader})
endforeach()

if(NOT WINDOWS_CI)
  enable_testing()
  message(STATUS "WINDOWS_CI is OFF or not defined.")
else()
  message(STATUS "WINDOWS_CI is OFF or not defined.")
endif()

add_subdirectory(Test/commit)
add_subdirectory(Test/compile)
add_subdirectory(Test/perf)
add_subdirectory(Test/fuzz)

# for correct library output needed
install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION bin)

# we keep the relative paths in the install dir
# keep in mind that install dir will be in front of this relative path ...
install(DIRECTORY Resources/ DESTINATION Resources/)
install(DIRECTORY ExternalLib/IMGUI/misc/fonts/ DESTINATION ExternalLib/IMGUI/misc/fonts/)

# we are using this dir strings for removing hard coded file locations
# at c++ side
configure_file(VulkanRendererConfig.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/VulkanRendererConfig.hpp")
configure_file(OpenGLRendererConfig.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/Src/GraphicsEngineOpenGL/renderer/OpenGLRendererConfig.hpp")

include(InstallRequiredSystemLibraries)

include(cmake/CPackOptions.cmake)

include(CPack)
