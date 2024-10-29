cmake_minimum_required(VERSION 3.28.3)

# set the project name
project(GraphicsEngine VERSION 1.3
        DESCRIPTION "Epic Graphics"
        HOMEPAGE_URL "https://jonasheinle.de/"
        LANGUAGES  CXX C)

set(AUTHOR "Jotrockenmitlocken")

# Set WINDOWS_CI to OFF by default
# WINDOWS CI is only able to verify successful compiling ... Windows ... lel
set(WINDOWS_CI OFF CACHE BOOL "Enable Windows CI build options")

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/ProjectOptions.cmake)

myproject_setup_options()

myproject_global_options()

myproject_local_options()

include(cmake/Doxygen.cmake)
enable_doxygen()

include(cmake/Speedup.cmake)

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
                                                myproject_warnings
                                                # enable sanitizers
                                                myproject_options
)

target_link_libraries(${PROJECT_NAME}   PRIVATE GSL
																								spdlog::spdlog)

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

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

include(CPack)