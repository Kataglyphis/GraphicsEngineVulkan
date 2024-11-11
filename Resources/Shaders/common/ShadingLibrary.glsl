#include "Matlib.glsl"
// #include "microfacet.glsl"

#ifndef SHADING_LIB
#define SHADING_LIB

vec3 Reinhards_tonemapping(vec3 color) { return color / (color + vec3(1.f)); }

vec3 gamma_correction(vec3 color) { return pow(color / (color + vec3(1.0f)), vec3(1.0f / 2.2f)); }

vec3 LambertDiffuse(vec3 ambient) { return ambient / PI; }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (86)
// float GGX_SMITH_LAMBDA(vec3 v, vec3 N, float roughness, float alphax, float alphay)
// {

//     float alphax2 = alphax * alphax;
//     float alphay2 = alphay * alphay;
//     // equation (80)
//     float alpha_0 = sqrt(Cos2Phi(v, N) * alphax2 + Sin2Phi(v, N) * alphay2);
//     float a = 1.f / (alpha_0 * TanTheta(v, N));

//     return (-1.f + sqrt(1.f + (1.f / pow(a, 2)))) / 2.f;
// }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (43)
// float G1_GGX_SMITH(vec3 v, vec3 N, float roughness, float alphax, float alphay)
// {

//     return 1.f / (1.f + GGX_SMITH_LAMBDA(v, N, roughness, alphax, alphay));
// }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (55)
// float G2_GGX_SMITH(vec3 wi, vec3 wo, vec3 N, float roughness, float alphax, float alphay)
// {

//     if (dot(wi, N) <= 0.f || dot(wo, N) <= 0.f) return 0.f;
//     return G1_GGX_SMITH(wi, N, roughness, alphax, alphay) * G1_GGX_SMITH(wo, N, roughness, alphax, alphay);
// }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (72)
// float GGX_SMITH_LAMBDA_ISOTROPIC(vec3 v, vec3 N, float roughness)
// {

//     float a = 1.f / (roughness * TanTheta(v, N));

//     return (-1.f + sqrt(1.f + (1.f / pow(a, 2)))) / 2.f;
// }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (43)
// float G1_GGX_SMITH_ISOTROPIC(vec3 wi, vec3 N, float roughness)
// {

//     return 1.f / (1.f + GGX_SMITH_LAMBDA_ISOTROPIC(wi, N, roughness));
// }

// // ref: https://hal.inria.fr/hal-01024289/
// // Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// // by Eric Heitz
// // equation (55)
// float G2_GGX_SMITH_ISOTROPIC(vec3 wi, vec3 wo, vec3 N, float roughness)
// {

//     if (dot(wi, N) <= 0.f || dot(wo, N) <= 0.f) return 0.f;
//     return G1_GGX_SMITH_ISOTROPIC(wi, N, roughness) * G1_GGX_SMITH_ISOTROPIC(wo, N, roughness);
// }


vec3 fresnel_schlick(float cosTheta, vec3 ambient_color, float metallic)
{

    vec3 F0 = mix(vec3(0.04), ambient_color, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}

// for the case u want to visualize depth
// otherwise u only look on white texture :)
float LinearizeDepth(float depth, float near_plane, float far_plane)
{
    float z = depth * 2.0 - 1.0;// Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}
#endif