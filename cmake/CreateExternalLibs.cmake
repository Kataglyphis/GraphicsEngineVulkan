add_library(
    IMGUI
    OBJECT
    ${IMGUI_FILTER}
)

target_link_libraries(IMGUI         PRIVATE Vulkan::Vulkan
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