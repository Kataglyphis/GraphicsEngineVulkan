# hint for the IDE which belongs together
source_group("gui"                                      FILES ${GUI_FILTER})
source_group("gui/imgui"                                FILES ${IMGUI_FILTER})

source_group("shaders/hostDevice"                       FILES ${HOST_DEVICE_SHADER_FILTER})
source_group("shaders/pbr"                              FILES ${PBR_SHADER_FILTER})
source_group("shaders/pbr/brdf"                         FILES ${BRDF_SHADER_FILTER})
source_group("shaders/common"                           FILES ${COMMON_SHADER_FILTER})
source_group("shaders/clouds"                           FILES ${CLOUDS_SHADER_FILTER})
source_group("shaders/loading_screen"                   FILES ${LOADING_SHADER_FILTER})
source_group("shaders/rasterizer"                       FILES ${RASTERIZER_SHADER_FILTER})
source_group("shaders/rasterizer/shadows"               FILES ${RASTERIZER_SHADOWS_SHADER_FILTER})
source_group("shaders/skybox"                           FILES ${SKYBOX_SHADER_FILTER})

source_group("compute"                                  FILES ${COMPUTE_FILTER})

source_group("camera"                                   FILES ${CAMERA_FILTER})

source_group("renderer"                                 FILES ${RENDERER_FILTER})
source_group("renderer/deferred"                        FILES ${DEFERRED_FILTER})
source_group("renderer/loading_screen"                  FILES ${LOAD_SCREEN_FILTER})

source_group("window"                                   FILES ${WINDOW_FILTER})
source_group("debug"                                    FILES ${DEBUG_FILTER})
source_group("util"                                     FILES ${UTIL_FILTER})

source_group("glad"                                     FILES ${GLAD_FILTER})

source_group("scene/atmospheric_effects/clouds"         FILES ${CLOUD_FILTER})

source_group("scene/lights"                             FILES ${LIGHT_FILTER})
source_group("scene/lights/directional"                 FILES ${D_LIGHT_FILTER})
source_group("scene/lights/point"                       FILES ${P_LIGHT_FILTER})

source_group("scene/shadows"                            FILES ${SHADOWS_FILTER})
source_group("scene/sky_box"                            FILES ${SKY_BOX_FILTER})
source_group("scene/texture"                            FILES ${TEXTURE_FILTER})
source_group("scene"                                    FILES ${SCENE_FILTER})

source_group("app"                                      FILES ${APP_FILTER})
source_group("common"                                   FILES ${COMMON_FILTER})