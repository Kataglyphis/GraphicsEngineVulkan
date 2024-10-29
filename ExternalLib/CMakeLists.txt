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

add_subdirectory(spdlog)