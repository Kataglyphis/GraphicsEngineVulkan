# setting all shader filters
# ---- SHADER FILTER  --- BEGIN

# ---- SHADER RASTERIZER FILTER  --- BEGIN
set(SHADER_RASTERIZER_SRC_DIR ${SHADER_SRC_DIR}rasterizer/)
set(RASTER_SHADER_FILTER ${RASTER_SHADER_FILTER} ${SHADER_RASTERIZER_SRC_DIR}shader.vert
                         ${SHADER_RASTERIZER_SRC_DIR}shader.frag)
# ---- SHADER RASTERIZER FILTER  --- END

# ---- SHADER RAYTRACING FILTER  --- BEGIN
set(SHADER_RAYTRACING_SRC_DIR ${SHADER_SRC_DIR}raytracing/)
set(RAYTRACING_SHADER_FILTER
    ${RAYTRACING_SHADER_FILTER}
    ${SHADER_RAYTRACING_SRC_DIR}raytrace.rchit
    ${SHADER_RAYTRACING_SRC_DIR}raytrace.rgen
    ${SHADER_RAYTRACING_SRC_DIR}raytrace.rmiss
    ${SHADER_RAYTRACING_SRC_DIR}shadow.rmiss)
# ---- SHADER RASTERIZER FILTER  --- END

# ---- SHADER COMMON FILTER  --- BEGIN
set(SHADER_COMMON_SRC_DIR ${SHADER_SRC_DIR}common/)
set(COMMON_SHADER_FILTER
    ${COMMON_SHADER_FILTER}
    ${SHADER_COMMON_SRC_DIR}Matlib.glsl
    ${SHADER_COMMON_SRC_DIR}microfacet.glsl
    ${SHADER_COMMON_SRC_DIR}raycommon.glsl
    ${SHADER_COMMON_SRC_DIR}ShadingLibrary.glsl)
# ---- SHADER COMMON FILTER  --- END

# ---- SHADER POST FILTER  --- BEGIN
set(SHADER_POST_SRC_DIR ${SHADER_SRC_DIR}post/)
set(POST_SHADER_FILTER ${POST_SHADER_FILTER} ${SHADER_POST_SRC_DIR}post.vert ${SHADER_POST_SRC_DIR}post.frag)
# ---- SHADER POST FILTER  --- END

# ---- SHADER PATH_TRACING FILTER  --- BEGIN
set(SHADER_PATH_TRACING_SRC_DIR ${SHADER_SRC_DIR}path_tracing/)
set(PATH_TRACING_SHADER_FILTER ${PATH_TRACING_SHADER_FILTER} ${SHADER_PATH_TRACING_SRC_DIR}path_tracing.comp)
# ---- SHADER PATH_TRACING FILTER  --- END

# ---- SHADER BRDF FILTER  --- BEGIN
set(SHADER_BRDF_SRC_DIR ${SHADER_SRC_DIR}brdf/)
set(BRDF_SHADER_FILTER
    ${BRDF_SHADER_FILTER}
    ${SHADER_BRDF_SRC_DIR}disney.glsl
    ${SHADER_BRDF_SRC_DIR}frostbite.glsl
    ${SHADER_BRDF_SRC_DIR}pbrBook.glsl
    ${SHADER_BRDF_SRC_DIR}phong.glsl
    ${SHADER_BRDF_SRC_DIR}unreal4.glsl)
# ---- SHADER BRDF FILTER  --- END

# ---- SHADER HOST_DEVICE FILTER  --- BEGIN
set(SHADER_HOST_DEVICE_SRC_DIR ${SHADER_SRC_DIR}hostDevice/)
set(SHADER_HOST_DEVICE_FILTER ${SHADER_HOST_DEVICE_FILTER} ${SHADER_HOST_DEVICE_SRC_DIR}host_device_shared_vars.hpp)
# ---- SHADER HOST_DEVICE FILTER  --- END

# ---- SHADER FILTER  --- END
