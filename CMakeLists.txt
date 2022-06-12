cmake_minimum_required(VERSION 3.18)

# set the project name
project(GraphicsEngine VERSION 1.3
        DESCRIPTION "Epic Graphics"
        HOMEPAGE_URL "https://jotrockenmitlocken.de/"
        LANGUAGES  CXX C)

# specify the C/C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

# link this library for using warnings specified in cmake/CmakeWarnings.cmake
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# add sanitizers if GCC/CLANG
add_library(project_sanitizers INTERFACE)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_sanitizers)

include(cmake/Doxygen.cmake)
enable_doxygen()

include(cmake/StaticAnalyzers.cmake)

include(FetchContent)
# include google test to project
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#include gsl
# In this example we are picking a specific tag.
# You can also pick a specific commit, if you need to.
FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.0.0"
)
FetchContent_MakeAvailable(GSL)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(ExternalLib)

# compile glslc shaders 
# source: https://www.reddit.com/r/vulkan/comments/kbaxlz/what_is_your_workflow_when_compiling_shader_files/
function(add_shader TARGET SHADER)
  find_program(GLSLC glslc)

  set(current-shader-path ${SHADER}) #${CMAKE_CURRENT_SOURCE_DIR}/
  get_filename_component(a_dir "${current-shader-path}" PATH)
  get_filename_component(a_last_dir "${current-shader-path}" NAME)

  set(current-output-path ${a_dir}/spv/${a_last_dir}.spv)
  # message(STATUS "${current-output-path}")

  get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
  file(MAKE_DIRECTORY ${current-output-dir})

  add_custom_command(
        OUTPUT ${current-output-path}
        COMMAND ${GLSLC} --target-env=vulkan1.3 -o ${current-output-path} ${current-shader-path}
        DEPENDS ${current-shader-path}
        IMPLICIT_DEPENDS CXX ${current-shader-path}
        VERBATIM)

  # Make sure our build depends on this output.
  set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
  target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

# we depend on vulkan 
find_package(Vulkan REQUIRED)
# configure vulkan version
set(VULKAN_VERSION_MAJOR 1)
set(VULKAN_VERSION_MINOR 3)
find_package(Threads REQUIRED)

# # so we can compile our shaders on the fly later on 
# add_compile_definitions(GLSLC_EXE="${Vulkan_GLSLC_EXECUTABLE}" )
# # remove dirty file directories ... 
# add_compile_definitions(CMAKELISTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

FILE(GLOB RENDERER_FILTER               "include/renderer/*.h"                                       "Src/renderer/*.cpp"
                                        "include/renderer/*.hpp")

FILE(GLOB PC_FILTER                     "include/renderer/pushConstants/*.h"                                   
                                        "include/renderer/pushConstants/*.hpp")

FILE(GLOB AS_FILTER                     "include/renderer/accelerationStructures/*.h"                "Src/renderer/accelerationStructures/*.cpp"
                                        "include/renderer/accelerationStructures/*.hpp")

FILE(GLOB VULKAN_BASE_FILTER            "include/vulkan_base/*.h"                                    "Src/vulkan_base/*.cpp"
                                        "include/vulkan_base/*.hpp")

FILE(GLOB SCENE_FILTER                  "include/scene/*.h"                                          "Src/scene/*.cpp"
                                        "include/scene/*.hpp")

FILE(GLOB WINDOW_FILTER                 "include/window/*.h"                                        "Src/window/*.cpp"
                                        "include/window/*.hpp")

FILE(GLOB MEMORY_FILTER                 "include/memory/*.h"                                        "Src/memory/*.cpp"
                                        "include/memory/*.hpp")

FILE(GLOB UTIL_FILTER                   "include/util/*.h"                                          "Src/util/*.cpp"
                                        "include/util/*.hpp")

FILE(GLOB COMMON_FILTER                 "include/common/*.h"                                          
                                        "include/common/*.hpp")

FILE(GLOB APP_FILTER                    "include/*.h"                                               "Src/*.cpp"
                                        "include/*.hpp")

FILE(GLOB SHADER_HOST_DEVICE_FILTER     "Resources/Shader/hostDevice/*.h"                     "Resources/Shader/hostDevice/*.cpp"
                                        "Resources/Shader/hostDevice/*.hpp")

# https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak
# https://stackoverflow.com/questions/45090926/cmake-generate-list-of-source-files-without-glob
# lets remove the FILE GLOB ... it is bad (as stated in the docs ... https://cmake.org/cmake/help/latest/command/file.html)

# update current positions
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src/)
set(EXTERNAL_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ExternalLib/)
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/)

include(cmake/filters/SetShaderFilters.cmake)
include(cmake/filters/SetProjectFilters.cmake)
include(cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetSourceGroups.cmake)

# add the executable
add_executable(${PROJECT_NAME}
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
${APP_FILTER}
${GUI_FILTER}
${IMGUI_FILTER}
${UTIL_FILTER}
${VULKAN_BASE_FILTER}
${SHADER_HOST_DEVICE_FILTER}
)

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

target_include_directories(${PROJECT_NAME} PUBLIC   ./include/renderer
                                                    ./include/renderer/pushConstants
                                                    ./include/renderer/accelerationStructures
                                                    ./include/vulkan_base
                                                    ./include/gui
                                                    ./include/common
                                                    ./include/scene
                                                    ./include/window
                                                    ./include/memory
                                                    ./include/util
                                                    ./Resources/Shader/hostDevice
                                                    ./include/ )

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}   PUBLIC  ${CMAKE_DL_LIBS}
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
                                                project_warnings
                                                # enable sanitizers
                                                project_sanitizers)

target_link_libraries(${PROJECT_NAME}   PRIVATE GSL)

enable_testing()

include(GoogleTest)

FILE(GLOB RENDERER_COMPILE_TEST_FILTER          "Test/compile/*.h"                                               "Test/compile/*.cpp"
                                                "Test/compile/*.hpp")

source_group("COMPILE_TEST_FILES"                                   FILES ${RENDERER_COMPILE_TEST_FILTER})

FILE(GLOB RENDERER_COMMIT_TEST_FILTER           "Test/commit/*.h"                                               "Test/commit/*.cpp"
                                                "Test/commit/*.hpp")

source_group("COMMIT_TEST_FILES"                                   FILES ${RENDERER_COMMIT_TEST_FILTER})

set (COMMIT_TEST_SUITE commitTestSuite)
set (COMPILE_TEST_SUITE compileTestSuite)

add_executable( ${COMMIT_TEST_SUITE}
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
                ${IMGUI_FILTER}
                ${UTIL_FILTER}
                ${VULKAN_BASE_FILTER}
                ${SHADER_HOST_DEVICE_FILTER}
)

add_executable( ${COMPILE_TEST_SUITE}
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
                ${IMGUI_FILTER}
                ${UTIL_FILTER}
                ${VULKAN_BASE_FILTER}
                ${SHADER_HOST_DEVICE_FILTER}
)

target_link_libraries( ${COMMIT_TEST_SUITE} PUBLIC ${CMAKE_DL_LIBS}
                                                    Threads::Threads
                                                    Vulkan::Vulkan
                                                    glfw 
                                                    imgui 
                                                    stb
                                                    glm
                                                    tinyobjloader
                                                    vma
                                                    ktx
                                                    gtest_main

)

target_link_libraries( ${COMPILE_TEST_SUITE}  PUBLIC ${CMAKE_DL_LIBS}
                                                      Threads::Threads
                                                      Vulkan::Vulkan
                                                      glfw 
                                                      imgui 
                                                      stb
                                                      glm
                                                      tinyobjloader
                                                      vma
                                                      ktx
                                                      gtest_main

)

target_link_libraries(${COMPILE_TEST_SUITE} PRIVATE GSL)
target_link_libraries(${COMMIT_TEST_SUITE} PRIVATE GSL)

target_include_directories(  ${COMPILE_TEST_SUITE} PUBLIC     ./include/renderer
                                                              ./include/renderer/pushConstants
                                                              ./include/renderer/accelerationStructures
                                                              ./include/vulkan_base
                                                              ./include/gui
                                                              ./include/common
                                                              ./include/scene
                                                              ./include/window
                                                              ./include/memory
                                                              ./include/util
                                                              ./Resources/Shader/hostDevice
                                                              ./include/
)

target_include_directories(  ${COMMIT_TEST_SUITE} PUBLIC    ./include/renderer
                                                            ./include/renderer/pushConstants
                                                            ./include/renderer/accelerationStructures
                                                            ./include/vulkan_base
                                                            ./include/gui
                                                            ./include/common
                                                            ./include/scene
                                                            ./include/window
                                                            ./include/memory
                                                            ./include/util
                                                            ./Resources/Shader/hostDevice
                                                            ./include/
)

target_include_directories(  ${COMPILE_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(  ${COMMIT_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})

gtest_discover_tests(   ${COMMIT_TEST_SUITE})
gtest_discover_tests(   ${COMPILE_TEST_SUITE})
# add test after each build
# src: https://quickturnstudio.com/running-test-in-compile-time/
add_custom_target(${COMPILE_TEST_SUITE}_Runner
                  COMMAND ${COMPILE_TEST_SUITE}
                  COMMENT "Running tests after each compilation.")

add_dependencies(${PROJECT_NAME} ${COMPILE_TEST_SUITE}_Runner)

# for correct library output needed
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin)

install(DIRECTORY Resources/ DESTINATION Resources/)
install(DIRECTORY ExternalLib/IMGUI/misc/fonts/ DESTINATION ExternalLib/IMGUI/misc/fonts/)

# we are using this dir strings for removing hard coded file locations
# at c++ side
configure_file(VulkanRendererConfig.h.in "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/VulkanRendererConfig.h")

include(InstallRequiredSystemLibraries)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")

set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /out
    /.*build.*
)

if(MSVC)
   set(CPACK_SOURCE_GENERATOR "NSIS")
   set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_SOURCE_DIR}/images/faviconNew.ico)
   set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
   set(CPACK_NSIS_HELP_LINK https://jotrockenmitlocken.de/)
   set(CPACK_NSIS_URL_INFO_ABOUT https://jotrockenmitlocken.de/)
   set(CPACK_NSIS_CONTACT https://jotrockenmitlocken.de/)
   set (CPACK_NSIS_MODIFY_PATH "ON")
 else() 
   set(CPACK_SOURCE_GENERATOR "TGZ")
 endif()

include(CPack)