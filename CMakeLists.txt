cmake_minimum_required(VERSION 3.10)

# set the project name
project(GraphicsEngine VERSION 1.0
                       DESCRIPTION "Epic Graphics"
                       LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# configure a header file to pass the version number only
configure_file(Src/Config.h.in Src/Config.h)

#set all your sources accordingly
file(GLOB SOURCES "Src/*.cpp"
                  "Src/*.c"
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/*.cpp
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/backends/imgui_impl_opengl3.cpp)

file(GLOB HEADERS "Src/*.h"
                  "Src/*.hpp"
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/*.h
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/backends/imgui_impl_glfw.h
                  ${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/backends/imgui_impl_opengl3.h)
# add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


# include the normal header file 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/../Src" 
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/GLM"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/STB"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/TOL"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/IMGUI/backends"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/GLEW/glew-2.2.0/include/GL"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/GLEW/glew-2.2.0/include"
                                           PUBLIC "${PROJECT_BINARY_DIR}/../ExternalLib/GLFW/glfw-3.3.4/include/GLFW")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

install(FILES "${PROJECT_BINARY_DIR}/Src/Config.h"
  DESTINATION include)

file(COPY Resources/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY Resources/Models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY Resources/Textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add_custom_command(
#   TARGET ${PROJECT_NAME} PRE_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#           ${CMAKE_SOURCE_DIR}/Resources/Shaders/*
#           ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)

# we depend on OpenGL
# find_package( OpenGL REQUIRED )
# target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

#surpress warnings
set(OpenGL_GL_PREFERENCE LEGACY)

# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ExternalLib/GLFW/glfw-3.3.4)

target_link_libraries(${PROJECT_NAME} glfw)

# set all glew dependencies
add_subdirectory(ExternalLib/GLEW/glew-2.2.0/build/cmake)
target_link_libraries(${PROJECT_NAME} glew)

# add tests 
include(CTest)

add_test(test1 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${GraphicsEngine_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GraphicsEngine_VERSION_MINOR}")

set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
                       ${PROJECT_BINARY_DIR}/bin )
set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                       ${PROJECT_BINARY_DIR}/bin )

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

if(UNIX)
	set(GCC_COMPILE_FLAGS "-pthread")
	#set(GCC_LINK_FLAGS  "-nostartfiles -g")

	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
	#set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")
else()
endif()


include(CPack)
