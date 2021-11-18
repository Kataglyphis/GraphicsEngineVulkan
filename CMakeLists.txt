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

# configure a header file to pass the version number only
# configure_file(Src/Config.h.in Src/Config.h)

# we depend on vulkan 
find_package(Vulkan REQUIRED)

#set all your sources accordingly
file(GLOB SOURCES "Src/*.cpp"
                  "Src/*.c"
)

file(GLOB HEADERS "include/*.h"
                  "include/*.hpp"
                  ${PROJECT_BINARY_DIR}/../ExternalLib/ASSIMP/assimp-5.0.1/include/*.h)

# add filters to the solution explorer for the shaders :))
file(GLOB SHADER_FILTER   
        ${PROJECT_BINARY_DIR}/../Resources/Shader/*.vert
        ${PROJECT_BINARY_DIR}/../Resources/Shader/*.frag
        ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rgen
        ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rchit
        ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rmiss)

source_group("Shader" FILES ${SHADER_FILTER})
source_group("Header" FILES ${HEADERS})

# add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILTER}) 

if(UNIX)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ./include
                                            PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/ASSIMP/assimp-5.0.1/include/assimp")

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

# add ASSIMP to our cmake build
add_subdirectory(ExternalLib/ASSIMP/assimp-5.0.1)
target_link_libraries(${PROJECT_NAME} assimp)

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
