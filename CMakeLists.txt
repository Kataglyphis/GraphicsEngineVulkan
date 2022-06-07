cmake_minimum_required(VERSION 3.18)

# specify the C/C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

# *,-abseil-*,-altera-*,-android-*,-boost-*,	bugprone-*,	cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,-darwin-*,-fuchsia-*,google-*,	hicpp-*,	-linuxkernel-*,	-llvm-*,	-llvmlibc-*,	misc-*,	modernize-*,	-mpi-*,	-objc-*,	-openmp-*,	performance-*,	portability-*,readability-*,-zircon-*

# add compile flags; as restrictive as possible
# for windows
# source: https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
set(MSVC_WARNINGS
          /W4 # All reasonable warnings
          /w14242  #'identfier': conversion from 'type1' to 'type1', possible loss of data
          /w14254  #'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
          /w14263  #'function': member function does not override any base class virtual member function
          /w14265  #'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
          /w14287  #'operator': unsigned/negative constant mismatch
          /we4289  #nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
          /w14296  #'operator': expression is always 'boolean_value'
          /w14311  #'variable': pointer truncation from 'type1' to 'type2'
          /w14545  #expression before comma evaluates to a function which is missing an argument list
          /w14546  #function call before comma missing argument list
          /w14547  #'operator': operator before comma has no effect; expected operator with side-effect
          /w14549  #'operator': operator before comma has no effect; did you intend 'operator'?
          /w14555  #expression has no effect; expected expression with side-effect
          /w14619  #pragma warning: there is no warning number 'number'
          /w14640  #Enable warning on thread un-safe static member initialization
          /w14826  #Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
          /w14905  #wide string literal cast to 'LPSTR'
          /w14906  #string literal cast to 'LPWSTR'
          /w14928  #illegal copy-initialization; more than one user-defined conversion has been implicitly applied
)

# CLANG
set(CLANG_WARNINGS
          #enable sanitizer
          -fsanitize=thread,memory,address,undefined
          -pedantic -Warn # on language extensions
          -Wall -Wextra # reasonable and standard
          -Wshadow # warn the user if a variable declaration shadows one from a parent context
          -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
          -Wold-style-cast # warn for c-style casts
          -Wcast-align # warn for potential performance problem casts
          -Wunused # warn on anything being unused
          -Woverloaded-virtual # warn if you overload (not override) a virtual function
          -Wpedantic # (all versions of GCC, Clang >= 3.2) warn if non-standard C++ is used
          -Wconversion # warn on type conversions that may lose data
          -Wsign-conversion # (Clang all versions, GCC >= 4.3) warn on sign conversions
          -Wmisleading-indentation # (only in GCC >= 6.0) warn if indentation implies blocks where blocks do not exist
          -Wduplicated-cond # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
          -Wduplicated-branches # (only in GCC >= 7.0) warn if if / else branches have duplicated code
          -Wlogical-op # (only in GCC) warn about logical operations being used where bitwise were probably wanted
          -Wnull-dereference # (only in GCC >= 6.0) warn if a null dereference is detected
          -Wuseless-cast # (only in GCC >= 4.8) warn if you perform a cast to the same type
          -Wdouble-promotion # (GCC >= 4.6, Clang >= 3.8) warn if float is implicit promoted to double
          -Wformat=2 # warn on security issues around functions that format output (ie printf)
          -Wlifetime #(only special branch of Clang currently) shows object lifetime issues
)

option(TREAT_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
if(TREAT_WARNINGS_AS_ERRORS)
  set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
  set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
endif()

# GCC 
set(GCC_WARNINGS ${CLANG_WARNINGS})

if(MSVC)
  set(PROJECT_WARNINGS ${MSVC_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(PROJECT_WARNINGS ${CLANG_WARNINGS})
else()
  set(PROJECT_WARNINGS ${GCC_WARNINGS})
endif()

# set the project name
project(GraphicsEngine VERSION 1.3
                       DESCRIPTION "Epic Graphics"
                       LANGUAGES  CXX
                                  C)

include(FetchContent)
# include google test to project
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(GTEST_OUTPUT )
FetchContent_MakeAvailable(googletest)

#include gsl
# In this example we are picking a specific tag.
# You can also pick a specific commit, if you need to.
FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.0.0"
)
FetchContent_MakeAvailable(GSL)

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

# so we can compile our shaders on the fly later on 
add_compile_definitions(GLSLC_EXE="${Vulkan_GLSLC_EXECUTABLE}" )

# remove dirty file directories ... 
add_compile_definitions(CMAKELISTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

add_subdirectory(ExternalLib)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# we are using this dir strings for removing hard coded file locations
# at c++ side
# add_compile_definitions(SHADER_SRC_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader")
# add_compile_definitions(SHADER_SPV_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/spv")
# add_compile_definitions(SHADER_LOG_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/log")

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

FILE(GLOB GUI_FILTER                    "include/gui/*.h"                                           "Src/gui/*.cpp")

FILE(GLOB IMGUI_FILTER                  "ExternalLib/IMGUI/*.h"                                     "ExternalLib/IMGUI/*.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"              "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
                                        "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h"            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp")

source_group("vulkan_base"                                  FILES ${VULKAN_BASE_FILTER})
source_group("gui"                                          FILES ${GUI_FILTER})
source_group("gui/imgui"                                    FILES ${IMGUI_FILTER})
source_group("renderer"                                     FILES ${RENDERER_FILTER})
source_group("renderer/pc"                                  FILES ${PC_FILTER})
source_group("renderer/AS"                                  FILES ${AS_FILTER})
source_group("scene"                                        FILES ${SCENE_FILTER})
source_group("window"                                       FILES ${WINDOW_FILTER})
source_group("memory"                                       FILES ${MEMORY_FILTER})
source_group("common"                                       FILES ${COMMON_FILTER})
source_group("app"                                          FILES ${APP_FILTER})
source_group("util"                                         FILES ${UTIL_FILTER})

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
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.vert
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.frag
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.glsl
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.comp
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.geom
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.rgen
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.rchit
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.rahit
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.rmiss
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/rasterizer/*.glsl
      )

file(GLOB RAYTRACING_SHADER_FILTER   
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.frag
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.glsl
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.comp
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.rgen
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.rchit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.rahit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.rmiss
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/raytracing/*.glsl
)

file(GLOB COMMON_SHADER_FILTER   
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.frag
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.glsl
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.comp
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.rgen
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.rchit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.rahit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.rmiss
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/common/*.glsl
)

file(GLOB POST_SHADER_FILTER   
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.frag
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.glsl
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.comp
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.rgen
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.rchit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.rahit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.rmiss
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/post/*.glsl
)

file(GLOB BRDF_SHADER_FILTER   
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.frag
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.glsl
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.comp
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.rgen
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.rchit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.rahit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.rmiss
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/brdf/*.glsl
)

file(GLOB PATH_TRACING_SHADER_FILTER   
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.frag
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.glsl
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.comp
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.rgen
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.rchit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.rahit
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.rmiss
      ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shader/path_tracing/*.glsl
)

source_group("shaders/rasterizer/" FILES ${RASTER_SHADER_FILTER})
source_group("shaders/raytracing/" FILES ${RAYTRACING_SHADER_FILTER})
source_group("shaders/common/" FILES ${COMMON_SHADER_FILTER})
source_group("shaders/post/" FILES ${POST_SHADER_FILTER})
source_group("shaders/brdf/" FILES ${BRDF_SHADER_FILTER})
source_group("shaders/path_tracing/" FILES ${PATH_TRACING_SHADER_FILTER})

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
)

target_compile_options(${PROJECT_NAME} INTERFACE ${PROJECT_WARNINGS})

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

if(UNIX)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

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
                                                    ./include/ )

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})


# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ExternalLib/GLFW)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${GraphicsEngine_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GraphicsEngine_VERSION_MINOR}")

set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
                       ${PROJECT_BINARY_DIR})

set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                       ${PROJECT_BINARY_DIR})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}   PUBLIC  ${CMAKE_DL_LIBS}
                                                Threads::Threads
                                                Vulkan::Vulkan
                                                glfw 
                                                imgui 
                                                stb
                                                glm
                                                tinyobjloader
                                                vma
                                                ktx)

target_link_libraries(${PROJECT_NAME}   PRIVATE GSL)

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

option(ENABLE_CPPCHECK "Enable cppcheck" OFF)#ON
if(ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --suppress=missingInclude --enable=all --suppressions-list=Documents/cppcheck/suppressions.txt)
  endif(CPPCHECK)
endif()

option(ENABLE_CLANGTIDY "Enable clangtidy" OFF)#ON
if(ENABLE_CLANGTIDY)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
  endif(CLANGTIDY)
endif()

enable_testing()

include(GoogleTest)

FILE(GLOB RENDERER_TEST_FILTER                  "Test/*.h"                                               "Test/*.cpp"
                                                "Test/*.hpp")

source_group("TEST_FILES"                                   FILES ${RENDERER_TEST_FILTER})

add_executable( vulkanRendererTester
                # all test specific files -- start
                ${RENDERER_TEST_FILTER}
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
)

target_link_libraries( vulkanRendererTester PUBLIC ${CMAKE_DL_LIBS}
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

target_link_libraries(vulkanRendererTester PRIVATE GSL)

target_include_directories(  vulkanRendererTester PUBLIC    ./include/renderer
                                                            ./include/renderer/pushConstants
                                                            ./include/renderer/accelerationStructures
                                                            ./include/vulkan_base
                                                            ./include/gui
                                                            ./include/common
                                                            ./include/scene
                                                            ./include/window
                                                            ./include/memory
                                                            ./include/util
                                                            ./include/
)

target_include_directories(  vulkanRendererTester PRIVATE ${Vulkan_INCLUDE_DIRS})

gtest_discover_tests(   vulkanRendererTester )

# include(CPack)
