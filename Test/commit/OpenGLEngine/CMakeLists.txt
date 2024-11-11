# all testing related stuff

set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineOpenGL)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}Src/GraphicsEngineOpenGL)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${PROJECT_SRC_DIR}cmake/filters/SetShaderFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetProjectFilters.cmake)
include(${PROJECT_SRC_DIR}cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetTestFilters.cmake)

set (COMMIT_TEST_SUITE commitTestSuite)

add_executable(
        ${COMMIT_TEST_SUITE}
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

target_include_directories(${COMMIT_TEST_SUITE}         PUBLIC ${PROJECT_INCLUDE_DIR}Src
                                                ${PROJECT_INCLUDE_DIR}Src/camera
                                                ${PROJECT_INCLUDE_DIR}Src/compute
                                                ${PROJECT_INCLUDE_DIR}Src/renderer
                                                ${PROJECT_INCLUDE_DIR}Src/renderer/loading_screen
                                                ${PROJECT_INCLUDE_DIR}Src/renderer/deferred
                                                ${PROJECT_INCLUDE_DIR}Src/debug
                                                ${PROJECT_INCLUDE_DIR}Src/window
                                                ${PROJECT_INCLUDE_DIR}Src/scene
                                                ${PROJECT_INCLUDE_DIR}Src/scene/atmospheric_effects/clouds
                                                ${PROJECT_INCLUDE_DIR}Src/scene/light
                                                ${PROJECT_INCLUDE_DIR}Src/scene/light/directional_light
                                                ${PROJECT_INCLUDE_DIR}Src/scene/light/point_light
                                                ${PROJECT_INCLUDE_DIR}Src/scene/shadows
                                                ${PROJECT_INCLUDE_DIR}Src/scene/sky_box
                                                ${PROJECT_INCLUDE_DIR}Src/scene/texture
                                                ${PROJECT_INCLUDE_DIR}Src/gui
                                                ${PROJECT_INCLUDE_DIR}Src/util
                                                ${PROJECT_INCLUDE_DIR}Resources/Shaders/hostDevice
                                                        ${OPENGL_INCLUDE_DIRS})

target_include_directories(${COMMIT_TEST_SUITE} PRIVATE ${OPENGL_INCLUDE_DIRS})

target_link_libraries(${COMMIT_TEST_SUITE}      PUBLIC  gtest_main
                                                        ${CMAKE_DL_LIBS} # for imgui
                                                        Threads::Threads
                                                        ${OPENGL_LIBRARIES}
                                                        glfw
                                                        imgui
                                                        stb 
                                                        glm 
                                                        tinyobjloader
                                                        glad)

target_link_libraries(${COMMIT_TEST_SUITE}   PRIVATE GSL)

gtest_discover_tests(   ${COMMIT_TEST_SUITE}
                        DISCOVERY_TIMEOUT 300)

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE -w)
else()
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE -w)
endif()