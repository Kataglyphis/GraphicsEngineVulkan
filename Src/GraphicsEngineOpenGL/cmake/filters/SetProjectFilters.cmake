# setting all project filters
# ---- PROJECT FILTER  --- BEGIN
# ---- GUI FILTER  --- BEGIN
set(PROJECT_GUI_SRC_DIR ${PROJECT_SRC_DIR}gui/)
set(GUI_FILTER ${GUI_FILTER}
    ${PROJECT_GUI_SRC_DIR}GUI.cpp
    ${PROJECT_GUI_SRC_DIR}GUI.hpp
)
# ---- GUI FILTER  --- END

# ---- COMPUTE FILTER  --- BEGIN
set(PROJECT_COMPUTE_SRC_DIR ${PROJECT_SRC_DIR}compute/)
set(COMPUTE_FILTER ${COMPUTE_FILTER}
    ${PROJECT_COMPUTE_SRC_DIR}ComputeShaderProgram.cpp
    ${PROJECT_COMPUTE_SRC_DIR}ComputeShaderProgram.hpp
)
# ---- COMPUTE FILTER  --- END

# ---- CAMERA FILTER  --- BEGIN
set(PROJECT_CAMERA_SRC_DIR ${PROJECT_SRC_DIR}camera/)
set(CAMERA_FILTER ${CAMERA_FILTER}
    ${PROJECT_CAMERA_SRC_DIR}Camera.cpp
    ${PROJECT_CAMERA_SRC_DIR}Camera.hpp
)
# ---- CAMERA FILTER  --- END

# ---- RENDERER FILTER  --- BEGIN
set(PROJECT_RENDERER_SRC_DIR ${PROJECT_SRC_DIR}renderer/)
set(RENDERER_FILTER ${RENDERER_FILTER}
    ${PROJECT_RENDERER_SRC_DIR}Renderer.cpp
    ${PROJECT_RENDERER_SRC_DIR}Renderer.hpp
    ${PROJECT_RENDERER_SRC_DIR}RenderPassSceneDependend.cpp
    ${PROJECT_RENDERER_SRC_DIR}RenderPassSceneDependend.hpp
    ${PROJECT_RENDERER_SRC_DIR}ShaderIncludes.cpp
    ${PROJECT_RENDERER_SRC_DIR}ShaderIncludes.hpp
    ${PROJECT_RENDERER_SRC_DIR}ShaderProgram.cpp
    ${PROJECT_RENDERER_SRC_DIR}ShaderProgram.hpp
    ${PROJECT_RENDERER_SRC_DIR}OpenGLRendererConfig.hpp
)
# ---- RENDERER FILTER  --- END

# ---- DEFERRED FILTER  --- BEGIN
set(PROJECT_DEFERRED_SRC_DIR ${PROJECT_SRC_DIR}renderer/deferred/)
set(DEFERRED_FILTER ${DEFERRED_FILTER}
    ${PROJECT_DEFERRED_SRC_DIR}GBuffer.cpp
    ${PROJECT_DEFERRED_SRC_DIR}GBuffer.hpp
    ${PROJECT_DEFERRED_SRC_DIR}GeometryPass.cpp
    ${PROJECT_DEFERRED_SRC_DIR}GeometryPass.hpp
    ${PROJECT_DEFERRED_SRC_DIR}GeometryPassShaderProgram.cpp
    ${PROJECT_DEFERRED_SRC_DIR}GeometryPassShaderProgram.hpp
    ${PROJECT_DEFERRED_SRC_DIR}LightingPass.cpp
    ${PROJECT_DEFERRED_SRC_DIR}LightingPass.hpp
    ${PROJECT_DEFERRED_SRC_DIR}LightingPassShaderProgram.cpp
    ${PROJECT_DEFERRED_SRC_DIR}LightingPassShaderProgram.hpp
    ${PROJECT_DEFERRED_SRC_DIR}RenderPass.hpp
)
# ---- DEFERRED FILTER  --- END

# ---- LOAD_SCREEN FILTER  --- BEGIN
set(PROJECT_LOAD_SCREEN_SRC_DIR ${PROJECT_SRC_DIR}renderer/loading_screen/)
set(LOAD_SCREEN_FILTER ${LOAD_SCREEN_FILTER}
    ${PROJECT_LOAD_SCREEN_SRC_DIR}LoadingScreen.cpp
    ${PROJECT_LOAD_SCREEN_SRC_DIR}LoadingScreen.hpp
)
# ---- LOAD_SCREEN FILTER  --- END

# ---- CLOUD FILTER  --- BEGIN
set(PROJECT_CLOUD_SRC_DIR ${PROJECT_SRC_DIR}scene/atmospheric_effects/clouds/)
set(CLOUD_FILTER ${CLOUD_FILTER}
    ${PROJECT_CLOUD_SRC_DIR}Clouds.cpp
    ${PROJECT_CLOUD_SRC_DIR}Clouds.hpp
    ${PROJECT_CLOUD_SRC_DIR}Noise.cpp
    ${PROJECT_CLOUD_SRC_DIR}Noise.hpp
    ${PROJECT_CLOUD_SRC_DIR}Perlin.cpp
    ${PROJECT_CLOUD_SRC_DIR}Perlin.hpp
)
# ---- CLOUD FILTER  --- END

# ---- LIGHT FILTER  --- BEGIN
set(PROJECT_LIGHT_SRC_DIR ${PROJECT_SRC_DIR}scene/light/)
set(LIGHT_FILTER ${LIGHT_FILTER}
    ${PROJECT_LIGHT_SRC_DIR}Light.cpp
    ${PROJECT_LIGHT_SRC_DIR}Light.hpp
)
# ---- LIGHT FILTER  --- END

# ---- D_LIGHT FILTER  --- BEGIN
set(PROJECT_D_LIGHT_SRC_DIR ${PROJECT_SRC_DIR}scene/light/directional_light/)
set(D_LIGHT_FILTER ${D_LIGHT_FILTER}
    ${PROJECT_D_LIGHT_SRC_DIR}CascadedShadowMap.cpp
    ${PROJECT_D_LIGHT_SRC_DIR}CascadedShadowMap.hpp
    ${PROJECT_D_LIGHT_SRC_DIR}DirectionalLight.cpp
    ${PROJECT_D_LIGHT_SRC_DIR}DirectionalLight.hpp
    ${PROJECT_D_LIGHT_SRC_DIR}DirectionalShadowMapPass.cpp
    ${PROJECT_D_LIGHT_SRC_DIR}DirectionalShadowMapPass.hpp
)
# ---- D_LIGHT FILTER  --- END

# ---- P_LIGHT FILTER  --- BEGIN
set(PROJECT_P_LIGHT_SRC_DIR ${PROJECT_SRC_DIR}scene/light/point_light/)
set(P_LIGHT_FILTER ${P_LIGHT_FILTER}
    ${PROJECT_P_LIGHT_SRC_DIR}OmniDirShadowMap.cpp
    ${PROJECT_P_LIGHT_SRC_DIR}OmniDirShadowMap.hpp
    ${PROJECT_P_LIGHT_SRC_DIR}OmniDirShadowShaderProgram.cpp
    ${PROJECT_P_LIGHT_SRC_DIR}OmniDirShadowShaderProgram.hpp
    ${PROJECT_P_LIGHT_SRC_DIR}OmniShadowMapPass.cpp
    ${PROJECT_P_LIGHT_SRC_DIR}OmniShadowMapPass.hpp
    ${PROJECT_P_LIGHT_SRC_DIR}PointLight.cpp
    ${PROJECT_P_LIGHT_SRC_DIR}PointLight.hpp
)
# ---- P_LIGHT FILTER  --- END

# ---- SHADOWS FILTER  --- BEGIN
set(PROJECT_SHADOWS_SRC_DIR ${PROJECT_SRC_DIR}scene/shadows/)
set(SHADOWS_FILTER ${SHADOWS_FILTER}
    ${PROJECT_SHADOWS_SRC_DIR}ShadowMap.cpp
    ${PROJECT_SHADOWS_SRC_DIR}ShadowMap.hpp
)
# ---- SHADOWS FILTER  --- END

# ---- SKY_BOX FILTER  --- BEGIN
set(PROJECT_SKY_BOX_SRC_DIR ${PROJECT_SRC_DIR}scene/sky_box/)
set(SKY_BOX_FILTER ${SKY_BOX_FILTER}
    ${PROJECT_SKY_BOX_SRC_DIR}SkyBox.cpp
    ${PROJECT_SKY_BOX_SRC_DIR}SkyBox.hpp
)
# ---- SKY_BOX FILTER  --- END

# ---- TEXTURE FILTER  --- BEGIN
set(PROJECT_TEXTURE_SRC_DIR ${PROJECT_SRC_DIR}scene/texture/)
set(TEXTURE_FILTER ${TEXTURE_FILTER}
    ${PROJECT_TEXTURE_SRC_DIR}Texture.cpp
    ${PROJECT_TEXTURE_SRC_DIR}Texture.hpp
    ${PROJECT_TEXTURE_SRC_DIR}ClampToEdgeMode.cpp
    ${PROJECT_TEXTURE_SRC_DIR}ClampToEdgeMode.hpp
    ${PROJECT_TEXTURE_SRC_DIR}MirroredRepeatMode.cpp
    ${PROJECT_TEXTURE_SRC_DIR}MirroredRepeatMode.hpp
    ${PROJECT_TEXTURE_SRC_DIR}RepeatMode.cpp
    ${PROJECT_TEXTURE_SRC_DIR}RepeatMode.hpp
    ${PROJECT_TEXTURE_SRC_DIR}TextureWrappingMode.hpp
)
# ---- TEXTURE FILTER  --- END

# ---- SCENE FILTER  --- BEGIN
set(PROJECT_SCENE_SRC_DIR ${PROJECT_SRC_DIR}scene/)
set(SCENE_FILTER ${SCENE_FILTER}
    ${PROJECT_SCENE_SRC_DIR}ViewFrustumCulling.cpp
    ${PROJECT_SCENE_SRC_DIR}ViewFrustumCulling.hpp
    ${PROJECT_SCENE_SRC_DIR}Vertex.hpp
    ${PROJECT_SCENE_SRC_DIR}Scene.cpp
    ${PROJECT_SCENE_SRC_DIR}Scene.hpp
    ${PROJECT_SCENE_SRC_DIR}Rotation.hpp
    ${PROJECT_SCENE_SRC_DIR}ViewFrustumCulling.cpp
    ${PROJECT_SCENE_SRC_DIR}ViewFrustumCulling.hpp
    ${PROJECT_SCENE_SRC_DIR}Quad.cpp
    ${PROJECT_SCENE_SRC_DIR}Quad.hpp
    ${PROJECT_SCENE_SRC_DIR}ObjMaterial.cpp
    ${PROJECT_SCENE_SRC_DIR}ObjMaterial.hpp
    ${PROJECT_SCENE_SRC_DIR}ObjLoader.cpp
    ${PROJECT_SCENE_SRC_DIR}ObjLoader.hpp
    ${PROJECT_SCENE_SRC_DIR}Model.cpp
    ${PROJECT_SCENE_SRC_DIR}Model.hpp
    ${PROJECT_SCENE_SRC_DIR}Mesh.cpp
    ${PROJECT_SCENE_SRC_DIR}Mesh.hpp
    ${PROJECT_SCENE_SRC_DIR}GameObject.cpp
    ${PROJECT_SCENE_SRC_DIR}GameObject.hpp
    ${PROJECT_SCENE_SRC_DIR}AABB.cpp
    ${PROJECT_SCENE_SRC_DIR}AABB.hpp
)
# ---- SCENE FILTER  --- END

# ---- WINDOW FILTER  --- BEGIN
set(PROJECT_WINDOW_SRC_DIR ${PROJECT_SRC_DIR}window/)
set(WINDOW_FILTER ${WINDOW_FILTER}
    ${PROJECT_WINDOW_SRC_DIR}Window.cpp
    ${PROJECT_WINDOW_SRC_DIR}Window.hpp
)
# ---- WINDOW FILTER  --- END

# ---- DEBUG FILTER  --- BEGIN
set(PROJECT_DEBUG_SRC_DIR ${PROJECT_SRC_DIR}debug/)
set(DEBUG_FILTER ${DEBUG_FILTER}
    ${PROJECT_DEBUG_SRC_DIR}DebugApp.cpp
    ${PROJECT_DEBUG_SRC_DIR}DebugApp.hpp
)
# ---- DEBUG FILTER  --- END

# ---- UTIL FILTER  --- BEGIN
set(PROJECT_UTIL_SRC_DIR ${PROJECT_SRC_DIR}util/)
set(UTIL_FILTER ${UTIL_FILTER}
    ${PROJECT_UTIL_SRC_DIR}File.cpp
    ${PROJECT_UTIL_SRC_DIR}File.hpp
    ${PROJECT_UTIL_SRC_DIR}RandomNumbers.cpp
    ${PROJECT_UTIL_SRC_DIR}RandomNumbers.hpp
)
# ---- UTIL FILTER  --- END

# ---- APP FILTER  --- BEGIN
set(PROJECT_APP_SRC_DIR ${PROJECT_SRC_DIR}app/)
set(APP_FILTER ${APP_FILTER}
    ${PROJECT_APP_SRC_DIR}App.cpp
)
# ---- APP FILTER  --- END

# ---- COMMON FILTER  --- BEGIN
set(PROJECT_COMMON_SRC_DIR ${PROJECT_SRC_DIR})
set(COMMON_FILTER ${COMMON_FILTER}
    
)
# ---- COMMON FILTER  --- END