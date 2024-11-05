# update current positions
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include/)
set(EXTERNAL_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}../../ExternalLib/)
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Shaders/)

include(../../cmake/filters/SetShaderFilters.cmake)
include(../../cmake/filters/SetProjectFilters.cmake)
include(../../cmake/filters/SetExternalLibsFilters.cmake)
include(../../cmake/SetSourceGroups.cmake)

# add the executable
add_executable(${PROJECT_NAME}  ${RASTER_SHADER_FILTER}
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
                                ${UTIL_FILTER}
                                ${VULKAN_BASE_FILTER}
                                ${MAIN_FILTER}
                                ${SHADER_HOST_DEVICE_FILTER}
                                # this is great; no CPPCHECK,CLANG_TIDY here
                                $<TARGET_OBJECTS:IMGUI>
)

target_include_directories(${PROJECT_NAME} PUBLIC   ${PROJECT_INCLUDE_DIR}renderer
																										${PROJECT_INCLUDE_DIR}renderer/pushConstants
																										${PROJECT_INCLUDE_DIR}renderer/accelerationStructures
                                                    ${PROJECT_INCLUDE_DIR}vulkan_base
                                                    ${PROJECT_INCLUDE_DIR}gui
                                                    ${PROJECT_INCLUDE_DIR}common
                                                    ${PROJECT_INCLUDE_DIR}scene
                                                    ${PROJECT_INCLUDE_DIR}window
                                                    ${PROJECT_INCLUDE_DIR}memory
                                                    ${PROJECT_INCLUDE_DIR}util
                                                    ${PROJECT_INCLUDE_DIR}app
                                                    ${SHADER_SRC_DIR}hostDevice
                                                    ${PROJECT_INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}   PUBLIC  ${CMAKE_DL_LIBS}
                                                Threads::Threads
                                                Vulkan::Vulkan
                                                glfw 
                                                imgui 
                                                stb
                                                glm
                                                tinyobjloader
                                                vma
                                                ktx
                                                # enable compiler warnings
                                                myproject_warnings
                                                # enable sanitizers
                                                myproject_options
)

target_link_libraries(${PROJECT_NAME}   PRIVATE GSL
																								spdlog::spdlog
																								nlohmann_json::nlohmann_json)
