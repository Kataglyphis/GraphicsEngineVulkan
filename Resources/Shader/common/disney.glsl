#include "ShadingLibrary.glsl"

// the famous disney principled brdf model
// based on their paper (https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf)
// brent burley even released some helpful shader code:
// https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf

vec3 DisneyDiffuse(vec3 ambient, vec3 L, vec3 V, vec3 N, float roughness) {

    vec3 wh = normalize(L) + normalize(V);
    wh = normalize(wh);

    float cosTheta_d = clamp(dot(normalize(V), wh), -1.0f, 1.0f);
    float F_D90 = 0.5f + 2.f * roughness * cosTheta_d * cosTheta_d;

    vec3 lambertDiffuse = LambertDiffuse(ambient);

    // even if lit by the backside or viewed from backside
    // we want some ambient light! 
    // catch it here to avoid brutal brightness because of 
    // following attentuation terms :))
    if (CosTheta(L, N) <= 0 || CosTheta(V, N) <= 0) {
        return 0.3f * lambertDiffuse;
    }

    float F_light = 1.f + (F_D90 - 1.f) * pow(1.f - CosTheta(L, N), 5);
    float F_view = 1.f + (F_D90 - 1.f) * pow(1.f - CosTheta(V, N), 5);

    return lambertDiffuse * F_light * F_view;

}

// the D function for their primary layer: GTR2aniso
float D_Disney(vec3 wh, vec3 N, float roughness, float alphax, float alphay) {

    float alphax2 = alphax * alphax;
    float alphay2 = alphay * alphay;

    float cos2Theta_h = Cos2Theta(wh, N);
    float h_dot_y = SinTheta(wh, N) * SinPhi(wh, N);
    float h_dot_x = SinTheta(wh, N) * CosPhi(wh, N);
    float h_dot_y2 = h_dot_y * h_dot_y;
    float h_dot_x2 = h_dot_x * h_dot_x;

    float denom3 = (h_dot_x2 / alphax2) + (h_dot_y2 / alphay2) + cos2Theta_h;

    return 1.f / (PI * alphax * alphay * denom3 * denom3);
}

float G_Disney(vec3 wi, vec3 wo, vec3 N, float roughness, float alphax, float alphay)
{
    return G2_GGX_SMITH(wi, wo, N, roughness, alphax, alphay);
}

vec3 evaluateDisneysPBR(vec3 ambient, vec3 N, vec3 L, vec3 V, float roughness, vec3 light_color, float light_intensity) {


    // add diffuse term
    // add diffuse term before checking negativ cosinus!
    // we want some diffuse light for the sun even if cosinus negative
    vec3 diffuse = DisneyDiffuse(ambient, L, V, N, roughness);

    // 1.) case: get lit by light from the backside
    // 2.) case: view it from the back
    // you also need to take care of the equal zero case; otherwise divide by zero problems
    if (CosTheta(L,N) <= 0 || CosTheta(V,N) <= 0) {
        //return diffuse;
        return diffuse;
    }

    // anisotropic is element of [0,1]; default value 0
    float anisotropic = 0.9;
    float metallic = 0.1f;
    // mapping to anisotropic alpha's; see paper
    float aspect = sqrt(1.f - 0.9f * anisotropic);
    float alphax = (roughness * roughness) / aspect;
    float alphay = (roughness * roughness) * aspect;

    //GTR2aniso
    vec3 wh = normalize(L+V);
    float D = D_Disney(wh, N, roughness, alphax, alphay);
    //smith ggx aniso
    float G = G_Disney(normalize(V), normalize(L), N, roughness, alphax, alphay);
    // simple fresnel schlick approx
    vec3 F = fresnel_schlick(CosTheta(wh, V), ambient, metallic);

    // add specular term  
    vec3 specular = light_color * light_intensity * evaluateCookTorrenceSpecularBRDF(D, G, F, CosTheta(L, N), CosTheta(V, N)) * CosTheta(L, N);

    return diffuse + specular; 
}
