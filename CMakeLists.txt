cmake_minimum_required(VERSION 3.14)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(GraphicsEngine VERSION 1.0
                       DESCRIPTION "Epic Graphics"
                       LANGUAGES CXX)

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

# we depend on vulkan 
find_package(Vulkan REQUIRED)

FILE(GLOB RENDERER_FILTER               "include/renderer/*.h"                                      "Src/renderer/*.cpp"
                                        "include/renderer/*.hpp")
FILE(GLOB APP_FILTER                    "include/*.h"                                               "Src/*.cpp"
                                        "include/*.hpp")
FILE(GLOB GUI_FILTER                    "ExternalLib/IMGUI/*.h"                                     "ExternalLib/IMGUI/*.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"              "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h"            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp")

source_group("gui"                                              FILES ${GUI_FILTER})
source_group("app/renderer"                                     FILES ${RENDERER_FILTER})
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
                  "include/*.hpp"
                  "include/renderer/*.h"
                  )

file(GLOB GUI_HEADERS 
            "ExternalLib/IMGUI/*.h"
            "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"
            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h")

# add filters to the solution explorer for the shaders :))
# add filters to the solution explorer for the shaders :))
file(GLOB SHADER_FILTER   
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.vert
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.frag
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.glsl
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.comp
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.geom
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rgen
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rchit
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rahit
            ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rmiss
      )

source_group("app/shaders" FILES ${SHADER_FILTER})

# add the executable
add_executable(${PROJECT_NAME}
${SHADER_FILTER}
${RENDERER_FILTER}
${APP_FILTER}
${GUI_FILTER}
)

if(UNIX)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC   ./include/renderer
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
add_subdirectory(ExternalLib/GLFW/glfw-3.3.4)

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
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_BINARY_DIR}/../Documents/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
