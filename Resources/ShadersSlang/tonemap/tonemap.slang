import aces;
import fullscreen;

// Rust/WebGPU tonemap: tonemap.wgsl is generated from this file; bindings match forward.rs's TonemapPass.

[vk::binding(0, 0)] Texture2D<float4> hdrTex;
[vk::binding(1, 0)] SamplerState hdrSampler;
[vk::binding(2, 0)] Texture2D<float4> bloomTex;
[vk::binding(4, 0)] Texture2D<float4> aoTex;

struct TonemapUniforms
{
    // x: bloom strength, y: SSAO strength, z: unused, w: 1.0 to encode sRGB here (web canvas).
    float4 params;
};
[vk::binding(3, 0)] ConstantBuffer<TonemapUniforms> tonemapUniforms;

// [adapted EV, target EV] from cs_reduce_exposure, the one exposure source (bloom reads it too).
[vk::binding(5, 0)] StructuredBuffer<float> exposureState;

// Returned directly: a copy into a local struct makes Slang emit FullscreenVsOut with two @location(0), which Tint rejects.
[shader("vertex")]
FullscreenVsOut vs_main(uint vid : SV_VertexID)
{
    return fullscreen_vs(vid);
}

[shader("fragment")]
float4 fs_main(FullscreenVsOut In) : SV_Target
{
    float3 hdr = hdrTex.Sample(hdrSampler, In.uv).rgb;
    float3 bloom = bloomTex.Sample(hdrSampler, In.uv).rgb;
    float aoRaw = aoTex.Sample(hdrSampler, In.uv).r;
    float ao = lerp(1.0, aoRaw, tonemapUniforms.params.y);
    float exposure = exp2(exposureState[0]);
    float3 mapped = aces_tonemap(hdr * ao * exposure + bloom * tonemapUniforms.params.x);

    // WebGPU canvases have no sRGB surface format, so the web encodes here (docs/source/webgpu-srgb-audit.md).
    if (tonemapUniforms.params.w > 0.5)
        return float4(linear_to_srgb(mapped), 1.0);
    return float4(mapped, 1.0);
}