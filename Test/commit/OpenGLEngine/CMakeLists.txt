# all testing related stuff

set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineOpenGL/)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineOpenGL/)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${PROJECT_SRC_DIR}cmake/filters/SetShaderFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetProjectFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetTestFilters.cmake)

set (COMMIT_TEST_SUITE_OPENGL commitTestSuiteOpenGL)

add_executable(
        ${COMMIT_TEST_SUITE_OPENGL}
        # all test specific files -- start
        ${RENDERER_COMMIT_TEST_FILTER}
        # all test specific files -- end
        ${PBR_SHADER_FILTER}
        ${BRDF_SHADER_FILTER} 
        ${CAMERA_FILTER} 
        ${COMMON_SHADER_FILTER}  
        ${CLOUDS_SHADER_FILTER}
        ${LOADING_SHADER_FILTER}
        ${RASTERIZER_SHADER_FILTER}
        ${RASTERIZER_SHADOWS_SHADER_FILTER}
        ${SKYBOX_SHADER_FILTER}
        ${GUI_FILTER}
        ${WINDOW_FILTER}
        ${DEBUG_FILTER}
        ${UTIL_FILTER}
        ${COMPUTE_FILTER}
        ${RENDERER_FILTER}
        ${DEFERRED_FILTER}
        ${LOAD_SCREEN_FILTER}
        ${ATMOSPHERE_EFFECTS_FILTER}
        ${LIGHT_FILTER}
        ${D_LIGHT_FILTER}
        ${P_LIGHT_FILTER}
        ${CLOUD_FILTER}
        ${SHADOWS_FILTER}
        ${SKY_BOX_FILTER}
        ${TEXTURE_FILTER}
        ${SCENE_FILTER}
        ${COMMON_FILTER}
        $<TARGET_OBJECTS:IMGUI>
        $<TARGET_OBJECTS:GLAD>
)

target_compile_definitions(${COMMIT_TEST_SUITE_OPENGL} PRIVATE
    RELATIVE_RESOURCE_PATH="/../Resources/"
)
# we are using this dir strings for removing hard coded file locations
# at c++ side
configure_file(${PROJECT_SRC_DIR}/OpenGLRendererConfig.hpp.in "${PROJECT_SRC_DIR}/renderer/OpenGLRendererConfig.hpp")

target_include_directories(${COMMIT_TEST_SUITE_OPENGL}         PUBLIC ${PROJECT_INCLUDE_DIR}
                                                ${PROJECT_INCLUDE_DIR}camera
                                                ${PROJECT_INCLUDE_DIR}compute
                                                ${PROJECT_INCLUDE_DIR}renderer
                                                ${PROJECT_INCLUDE_DIR}renderer/loading_screen
                                                ${PROJECT_INCLUDE_DIR}renderer/deferred
                                                ${PROJECT_INCLUDE_DIR}debug
                                                ${PROJECT_INCLUDE_DIR}window
                                                ${PROJECT_INCLUDE_DIR}scene
                                                ${PROJECT_INCLUDE_DIR}scene/atmospheric_effects/clouds
                                                ${PROJECT_INCLUDE_DIR}scene/light
                                                ${PROJECT_INCLUDE_DIR}scene/light/directional_light
                                                ${PROJECT_INCLUDE_DIR}scene/light/point_light
                                                ${PROJECT_INCLUDE_DIR}scene/shadows
                                                ${PROJECT_INCLUDE_DIR}scene/sky_box
                                                ${PROJECT_INCLUDE_DIR}scene/texture
                                                ${PROJECT_INCLUDE_DIR}gui
                                                ${PROJECT_INCLUDE_DIR}util
                                                ${PROJECT_INCLUDE_DIR}Resources/Shaders/hostDevice
                                                ${OPENGL_INCLUDE_DIRS}
                                                ${SHADER_SRC_DIR}hostDevice)

target_include_directories(${COMMIT_TEST_SUITE_OPENGL} PRIVATE ${OPENGL_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${COMMIT_TEST_SUITE_OPENGL}      PUBLIC  gtest_main
                                                        ${CMAKE_DL_LIBS} # for imgui
                                                        Threads::Threads
                                                        ${OPENGL_LIBRARIES}
                                                        Vulkan::Vulkan
                                                        glfw
                                                        imgui
                                                        stb 
                                                        glm 
                                                        tinyobjloader
                                                        glad)

target_link_libraries(${COMMIT_TEST_SUITE_OPENGL}   PRIVATE GSL)

gtest_discover_tests(   ${COMMIT_TEST_SUITE_OPENGL}
                        DISCOVERY_TIMEOUT 300)

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMMIT_TEST_SUITE_OPENGL}  INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMMIT_TEST_SUITE_OPENGL}  INTERFACE -w)
else()
  target_compile_options(${COMMIT_TEST_SUITE_OPENGL}  INTERFACE -w)
endif()