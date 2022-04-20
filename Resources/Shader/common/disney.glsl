#include "ShadingLibrary.glsl"

// (https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf)

vec3 DisneyDiffuse(vec3 ambient, vec3 L, vec3 V, vec3 N, float roughness) {

    vec3 wo = normalize(L);
    vec3 wi = normalize(V);
    vec3 wh = wi + wo;
    wh = normalize(wh);

    float cosTheta_d = clamp(dot(wi, wh), -1.0f, 1.0f);
    float F_D90 = 0.5f + 2.f * roughness * cosTheta_d * cosTheta_d;

    vec3 lambertDiffuse = LambertDiffuse(ambient);

    float cosTheta_l = CosTheta(L, N);
    float cosTheta_v = CosTheta(V, N);
    float F_light = 1.f + (F_D90 - 1.f) * pow(1.f - cosTheta_l, 5);
    float F_view = 1.f + (F_D90 - 1.f) * pow(1.f - cosTheta_v, 5);

    return lambertDiffuse * F_light * F_view;

}

float D_Disney(vec3 wh, vec3 N, float roughness) {

    //GTR2 anisotropic
    float anisotropic = 0;
    float aspect = sqrt(1.f - 0.9f * anisotropic);
    float alphax = (roughness * roughness) / aspect;
    float alphay = (roughness * roughness) * aspect;
    float alphax2 = alphax * alphax;
    float alphay2 = alphay * alphay;

    float cos2Theta_h = Cos2Theta(wh, N);
    float h_dot_y = SinTheta(wh, N) * SinPhi(wh, N);
    float h_dot_x = SinTheta(wh, N) * CosPhi(wh, N);

    float denom3 = ((h_dot_x * h_dot_x) / alphax2) + ((h_dot_y * h_dot_y) / alphay2) + cos2Theta_h;

    return 1.f / (PI * alphax * alphay * denom3 * denom3);
}

float G_Disney(vec3 wi, vec3 wo, vec3 N, float roughness)
{
    vec3 wh = normalize(wi + wo);
    if (dot(wi, N) <= 0.f || dot(wo, N) <= 0.f) return 0.f;
    return 1.f / (1.f + Lamda_heitz_2014(wi, N, roughness) + Lamda_heitz_2014(wo, N, roughness));

}

vec3 evaluateDisneysPBR(vec3 ambient, vec3 N, vec3 L, vec3 V, float roughness, vec3 light_color, float light_intensity) {

    // add lambertian diffuse term
    vec3 color = DisneyDiffuse(ambient, L, V, N, roughness);

    vec3 wo = normalize(L);
    vec3 wi = normalize(V);

    float cosThetaO = AbsCosTheta(wo, N);
    float cosThetaI = AbsCosTheta(wi, N);
    vec3 wh = wi + wo;
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0);
    wh = normalize(wh);
    float D = 1;
    float G = 1;
    vec3 F = vec3(1);

    D = D_Disney(wh, N, roughness);
    G = G_Disney(wi, wo, N, roughness);
    F = fresnel_schlick(CosTheta(wh, wi), ambient, 0.1);

    // add specular term  
    float cosTheta_l = CosTheta(L, N);
    float cosTheta_v = CosTheta(V, N);
    if (cosTheta_l > 0 && cosTheta_v > 0) {
        color += light_color * light_intensity * evaluateCookTorrenceSpecularBRDF(D, G, F, cosTheta_l, cosTheta_v) * cosTheta_l;
    }

    return color;

}
