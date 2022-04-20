#include "Matlib.glsl"
#include "microfacet.glsl"

#ifndef SHADING_LIB
#define SHADING_LIB
vec3 LambertDiffuse(vec3 ambient) {

    return ambient / PI;

}

float Lamda_heitz_2014(vec3 v, vec3 N, float roughness) {

    float anisotropic = 0;
    float aspect = sqrt(1.f - 0.9f * anisotropic);
    float alphax = (roughness * roughness) / aspect;
    float alphay = (roughness * roughness) * aspect;

    float alphax2 = alphax * alphax;
    float alphay2 = alphay * alphay;

    float alpha_0 = sqrt(Cos2Phi(v,N) * alphax2 + Sin2Phi(v, N) * alphay2);
    float a = 1.f / (alpha_0 * TanTheta(v,N));
    
    return (-1.f + sqrt(1.f + (1.f / pow(a,2)))) / 2.f;

}

vec3 fresnel_schlick(float cosTheta, vec3 ambient_color, float metallic) {

    vec3 F0 = mix(vec3(0.04), ambient_color, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;

}
#endif