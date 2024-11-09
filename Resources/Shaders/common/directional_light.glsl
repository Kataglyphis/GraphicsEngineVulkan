#ifndef D_LIGHT
#define D_LIGHT

#include "./light.glsl"

struct DirectionalLight {

    Light   base;
    vec3    direction;
    float   shadow_intensity;

};

//with PCF; cascaded approach
float calc_directional_shadow_factor(   DirectionalLight d_light, 
                                        sampler2DArray directional_shadow_maps,
                                        int pcf_radius,
                                        vec4 frag_pos, vec3 N, mat4 view, 
                                        float cascade_endpoints[NUM_CASCADES],
                                        mat4 lightSpaceMatrices[NUM_CASCADES]) {

    vec4 fragPosWorldSpace = vec4(frag_pos.xyz, 1.0f);
    vec4 fragPosViewSpace = view * fragPosWorldSpace;
    float frag_depth = abs(fragPosViewSpace.z);
    int cascade_index = -1;

    for (int i = 0; i < NUM_CASCADES; i++) {

        if (frag_depth < cascade_endpoints[i]) {
            cascade_index = i;
            break;
        }

    }

    if (cascade_index == -1) cascade_index = NUM_CASCADES;

    vec4 fragPosLightSpace = lightSpaceMatrices[cascade_index] * fragPosWorldSpace;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }

    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(N, d_light.direction)), 0.005);
    const float biasModifier = 0.5f;
    if (cascade_index == NUM_CASCADES)
    {
        float farPlane = cascade_endpoints[NUM_CASCADES - 1];
        bias *= 1 / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascade_endpoints[cascade_index] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    float numSamples = 0.0f;
    vec2 texelSize = 1.0 / vec2(textureSize(directional_shadow_maps, 0));
    for (int x = -pcf_radius; x <= pcf_radius; ++x)
    {
        for (int y = -pcf_radius; y <= pcf_radius; ++y)
        {
            float pcfDepth = texture(directional_shadow_maps, vec3(projCoords.xy + vec2(x, y) * texelSize, cascade_index)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
            numSamples++;
        }
    }

    shadow /= numSamples;

    return shadow;
}


#endif