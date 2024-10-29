cmake_minimum_required(VERSION 3.28.3)

# set the project name
project(GraphicsEngine VERSION 1.3
        DESCRIPTION "Epic Graphics"
        HOMEPAGE_URL "https://jonasheinle.de/"
        LANGUAGES  CXX C)

set(AUTHOR "Jotrockenmitlocken")

set(WINDOWS_CI OFF)

# set build type specific flags
if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
  # https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
  # https://clang.llvm.org/docs/UsersManual.html
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /O0")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 -DNDEBUG")
  # https://clang.llvm.org/docs/ClangCommandLineReference.html
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# if no build type passed via command line go with debug mode
if(NOT CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)   
endif()

# specify the C/C++ standard
set(CMAKE_CXX_STANDARD 23)
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

add_library(ipo INTERFACE)
include(cmake/Optimizations.cmake)
enable_ipo_for_target(ipo)

include(cmake/Doxygen.cmake)
enable_doxygen()

include(cmake/StaticAnalyzers.cmake)

include(cmake/Speedup.cmake)

include(FetchContent)
# include google test to project
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/df1544bcee0c7ce35cd5ea0b3eb8cc81855a4140.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#include gsl
# In this example we are picking a specific tag.
# You can also pick a specific commit, if you need to.
FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.1.0"
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

# https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak
# https://stackoverflow.com/questions/45090926/cmake-generate-list-of-source-files-without-glob
# lets remove the FILE GLOB ... it is bad (as stated in the docs ... https://cmake.org/cmake/help/latest/command/file.html)

# update current positions
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src/)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(EXTERNAL_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ExternalLib/)
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/)

include(cmake/filters/SetShaderFilters.cmake)
include(cmake/filters/SetProjectFilters.cmake)
include(cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetSourceGroups.cmake)

# https://stackoverflow.com/questions/49591804/clang-tidy-cmake-exclude-file-from-check/49591908#49591908
# with this trick IMGUI will be excluded from time consuming static code analysis
# we do not want to analyze imgui ...
# include(cmake/CreateProjectFilesToTestLib.cmake)
# and we will not to all this for our testing executable again :)
include(cmake/CreateExternalLibs.cmake)

# add the executable
add_executable(${PROJECT_NAME}  ${RASTER_SHADER_FILTER}
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
                                ${UTIL_FILTER}
                                ${VULKAN_BASE_FILTER}
                                ${MAIN_FILTER}
                                ${SHADER_HOST_DEVICE_FILTER}
                                # this is great; no CPPCHECK,CLANG_TIDY here
                                $<TARGET_OBJECTS:IMGUI>
)

enable_cppcheck(${PROJECT_NAME})
enable_clangtidy(${PROJECT_NAME})

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
                                                    ./include/app
                                                    ./Resources/Shaders/hostDevice
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
                                                project_sanitizers
                                                # enable ipo
                                                ipo)

target_link_libraries(${PROJECT_NAME}   PRIVATE GSL)

if(NOT WINDOWS_CI)
	enable_testing()
	message(STATUS "WINDOWS_CI is OFF or not defined.")
else()
	message(STATUS "WINDOWS_CI is OFF or not defined.")
endif()

include(GoogleTest)

add_subdirectory(Test/commit)
add_subdirectory(Test/compile)

# for correct library output needed
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin)
        
# we keep the relative paths in the install dir
# keep in mind that install dir will be in front of this relative path ... 
install(DIRECTORY Resources/ DESTINATION Resources/)
install(DIRECTORY ExternalLib/IMGUI/misc/fonts/ DESTINATION ExternalLib/IMGUI/misc/fonts/)

# we are using this dir strings for removing hard coded file locations
# at c++ side
configure_file(VulkanRendererConfig.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/VulkanRendererConfig.hpp")

include(InstallRequiredSystemLibraries)

include(cmake/CPackOptions.cmake)

include(CPack)