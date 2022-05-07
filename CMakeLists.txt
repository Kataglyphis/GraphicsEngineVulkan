cmake_minimum_required(VERSION 3.16)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(GraphicsEngine VERSION 1.0
                       DESCRIPTION "Epic Graphics"
                       LANGUAGES CXX)

# include google test to project
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/f079775276a99fd4373569bf5761052a01844270.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# compile glslc shaders 
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

add_compile_definitions(GLSLC_EXE="${Vulkan_GLSLC_EXECUTABLE}" )

add_subdirectory(ExternalLib)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


add_compile_definitions(SHADER_SRC_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader")
add_compile_definitions(SHADER_SPV_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/spv")
add_compile_definitions(SHADER_LOG_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/log")

FILE(GLOB RENDERER_FILTER               "include/renderer/*.h"                                      "Src/renderer/*.cpp"
                                        "include/renderer/*.hpp")

FILE(GLOB SCENE_FILTER                  "include/scene/*.h"                                           "Src/scene/*.cpp"
                                        "include/scene/*.hpp")

FILE(GLOB WINDOW_FILTER                 "include/window/*.h"                                         "Src/window/*.cpp"
                                        "include/window/*.hpp")

FILE(GLOB MEMORY_FILTER                 "include/memory/*.h"                                         "Src/memory/*.cpp"
                                        "include/memory/*.hpp")

FILE(GLOB COMMON_FILTER                 "include/common/*.h"                                          
                                        "include/common/*.hpp")

FILE(GLOB APP_FILTER                    "include/*.h"                                                 "Src/*.cpp"
                                        "include/*.hpp")

FILE(GLOB GUI_FILTER                    "ExternalLib/IMGUI/*.h"                                     "ExternalLib/IMGUI/*.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"              "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h"            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp")

source_group("gui"                                              FILES ${GUI_FILTER})
source_group("app/renderer"                                     FILES ${RENDERER_FILTER})
source_group("app/scene"                                        FILES ${SCENE_FILTER})
source_group("app/window"                                       FILES ${WINDOW_FILTER})
source_group("app/memory"                                       FILES ${MEMORY_FILTER})
source_group("app/common"                                       FILES ${COMMON_FILTER})
source_group("app"                                              FILES ${APP_FILTER})

#set all your sources accordingly
file(GLOB_RECURSE SOURCES 
            "Src/*.cpp" 
            "Src/*.c"
            "ExternalLib/IMGUI/imgui.cpp"
            "ExternalLib/IMGUI/imgui_demo.cpp"
            "ExternalLib/IMGUI/imgui_draw.cpp"
            "ExternalLib/IMGUI/imgui_tables.cpp"
            "ExternalLib/IMGUI/imgui_widgets.cpp"
            "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp")

file(GLOB_RECURSE HEADERS  
                  "include/*.h"
                  "include/*.hpp")

file(GLOB GUI_HEADERS 
            "ExternalLib/IMGUI/*.h"
            "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"
            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h")

# add filters to the solution explorer for the shaders :))
file(GLOB RASTER_SHADER_FILTER   
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.vert
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.frag
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.glsl
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.comp
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.geom
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.rgen
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.rchit
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.rahit
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.rmiss
            ${PROJECT_BINARY_DIR}/../Resources/Shader/rasterizer/*.glsl
      )

file(GLOB RAYTRACING_SHADER_FILTER   
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.vert
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.frag
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.glsl
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.comp
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.geom
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.rgen
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.rchit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.rahit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.rmiss
      ${PROJECT_BINARY_DIR}/../Resources/Shader/raytracing/*.glsl
)

file(GLOB COMMON_SHADER_FILTER   
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.vert
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.frag
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.glsl
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.comp
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.geom
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.rgen
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.rchit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.rahit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.rmiss
      ${PROJECT_BINARY_DIR}/../Resources/Shader/common/*.glsl
)

file(GLOB POST_SHADER_FILTER   
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.vert
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.frag
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.glsl
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.comp
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.geom
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.rgen
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.rchit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.rahit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.rmiss
      ${PROJECT_BINARY_DIR}/../Resources/Shader/post/*.glsl
)

file(GLOB BRDF_SHADER_FILTER   
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.vert
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.frag
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.glsl
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.comp
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.geom
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.rgen
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.rchit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.rahit
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.rmiss
      ${PROJECT_BINARY_DIR}/../Resources/Shader/brdf/*.glsl
)

source_group("app/shaders/rasterizer/" FILES ${RASTER_SHADER_FILTER})
source_group("app/shaders/raytracing/" FILES ${RAYTRACING_SHADER_FILTER})
source_group("app/shaders/common/" FILES ${COMMON_SHADER_FILTER})
source_group("app/shaders/post/" FILES ${POST_SHADER_FILTER})
source_group("app/shaders/brdf/" FILES ${BRDF_SHADER_FILTER})

# add the executable
add_executable(${PROJECT_NAME}
${RASTER_SHADER_FILTER}
${RAYTRACING_SHADER_FILTER}
${COMMON_SHADER_FILTER}
${POST_SHADER_FILTER}
${BRDF_SHADER_FILTER}
${RENDERER_FILTER}
${SCENE_FILTER}
${WINDOW_FILTER}
${MEMORY_FILTER}
${COMMON_FILTER}
${APP_FILTER}
${GUI_FILTER}
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

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

if(UNIX)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC   ./include/renderer
                                                    ./include/common
                                                    ./include/scene
                                                    ./include/window
                                                    ./include/memory
                                                    ./include/ )

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

# file(COPY Resources/Shader DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resources/Models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resources/Textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add_custom_command(
#   TARGET ${PROJECT_NAME} PRE_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#           ${CMAKE_SOURCE_DIR}/Resources/Shaders/*
#           ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)


# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ExternalLib/GLFW)

target_link_libraries(${PROJECT_NAME} glfw)

# add tests 
# include(CTest)

# add_test(test1 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${GraphicsEngine_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GraphicsEngine_VERSION_MINOR}")

set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
                       ${PROJECT_BINARY_DIR})

set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                       ${PROJECT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan imgui stb glm tinyobjloader vma ktx)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS}
                                      Threads::Threads)


if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
else()
endif()

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
# find_package(Doxygen)
# if (DOXYGEN_FOUND)
#     # set input and output files
#     set(DOXYGEN_IN ${CMAKE_CURRENT_BINARY_DIR}/../Documents/Doxyfile.in)
#     set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#     # request to configure the file
#     configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
#     message("Doxygen build started")

#     # note the option ALL which allows to build the docs together with the application
#     add_custom_target( doc_doxygen ALL
#         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         COMMENT "Generating API documentation with Doxygen"
#         VERBATIM )
# else (DOXYGEN_FOUND)
#   message("Doxygen need to be installed to generate the doxygen documentation")
# endif (DOXYGEN_FOUND)

enable_testing()

add_executable(
  hello_test
  Test/hello_test.cc
)
target_link_libraries(
  hello_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)

# include(CPack)
