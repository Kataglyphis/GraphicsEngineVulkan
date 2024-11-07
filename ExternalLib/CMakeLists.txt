include(FetchContent)

# hotfix for issue: 
# https://github.com/google/benchmark/pull/1875
add_compile_options(-Wno-c++17-attribute-extensions)

# include google test to project
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/df1544bcee0c7ce35cd5ea0b3eb8cc81855a4140.zip
)
  
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
    
add_subdirectory(GOOGLE_BENCHMARK)

# hotfix for issue: 
# https://github.com/google/benchmark/pull/1875
target_compile_options(benchmark INTERFACE -Wno-c++17-attribute-extensions)
target_compile_options(benchmark_main INTERFACE -Wno-c++17-attribute-extensions)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "This is a Linux system.")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      add_subdirectory(FUZZTEST)
      if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(FUZZTEST_FUZZING_MODE ON CACHE BOOL "" FORCE)
      endif()
    endif()
endif()

FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.1.0"
)
FetchContent_MakeAvailable(GSL)

# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


add_library(tinyobjloader INTERFACE)
# this helps us so no error messages for third party will be generated:
# https://stackoverflow.com/questions/49591804/clang-tidy-cmake-exclude-file-from-check
target_include_directories(tinyobjloader    SYSTEM 
                                            INTERFACE 
                                            TINY_OBJ_LOADER)

add_library(stb INTERFACE)
target_include_directories(stb  SYSTEM 
                                INTERFACE 
                                STB)

add_library(imgui INTERFACE)
target_include_directories(imgui    SYSTEM     
                                    INTERFACE 
                                    IMGUI IMGUI/backends)

add_library(glm INTERFACE)
target_include_directories(glm  SYSTEM 
                                INTERFACE 
                                GLM)

add_library(vma INTERFACE)
target_include_directories(vma  SYSTEM 
                                INTERFACE 
                                VULKAN_MEMORY_ALLOCATOR/include)


add_library(ktx INTERFACE)
target_include_directories(ktx  SYSTEM
                                INTERFACE    
                                KTX/include KTX/other_include/KHR)

add_subdirectory(GLFW)

add_subdirectory(SPDLOG)

add_subdirectory(NLOHMANN_JSON)

# https://stackoverflow.com/questions/49591804/clang-tidy-cmake-exclude-file-from-check/49591908#49591908
# with this trick IMGUI will be excluded from time consuming static code analysis
# we do not want to analyze imgui ...
# include(cmake/CreateProjectFilesToTestLib.cmake)
# and we will not to all this for our testing executable again :)
add_library(
    IMGUI
    OBJECT
    ${IMGUI_FILTER}
)

target_link_libraries(IMGUI         PRIVATE 
																						${CMAKE_DL_LIBS}
                                            Vulkan::Vulkan
                                            glfw 
                                            imgui

)

target_include_directories(IMGUI    PRIVATE ${Vulkan_INCLUDE_DIRS})

set_target_properties(
    IMGUI
    PROPERTIES
    CXX_CLANG_TIDY ""
    CXX_CPPCHECK ""
)
