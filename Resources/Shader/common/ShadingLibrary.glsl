#include "Matlib.glsl"

vec3 LambertDiffuse(vec3 ambient) {

    return ambient / PI;

}

vec3 DisneyDiffuse(vec3 ambient, vec3 L, vec3 V, vec3 N, float roughness) {

    vec3 wo = normalize(L);
    vec3 wi = normalize(V);
    vec3 wh = wi + wo;
    wh = normalize(wh);

    float cosTheta_d = clamp(dot(wi, wh), -1.0f, 1.0f);
    float F_D90 = 0.5f + 2.f * roughness * cosTheta_d * cosTheta_d;

    vec3 lambertDiffuse = LambertDiffuse(ambient);

    float cosTheta_l = CosTheta(L,N);
    float cosTheta_v = CosTheta(V,N);
    float F_light = 1.f + (F_D90 - 1.f) * pow(1.f - cosTheta_l,5);
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

    float cos2Theta_h = Cos2Theta(wh,N);
    float h_dot_y = SinTheta(wh,N)*SinPhi(wh,N);
    float h_dot_x = SinTheta(wh,N)*CosPhi(wh,N);

    float denom3 = ((h_dot_x * h_dot_x) / alphax2) + ((h_dot_y * h_dot_y) / alphay2) + cos2Theta_h;

    return 1.f / (PI * alphax * alphay * denom3 * denom3);
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

float G_Disney(vec3 wi, vec3 wo, vec3 N, float roughness)
{
    vec3 wh = normalize(wi+wo);
    if (dot(wi,N) <= 0.f || dot(wo,N) <= 0.f) return 0.f;
    return 1.f / (1.f + Lamda_heitz_2014(wi,N,roughness) + Lamda_heitz_2014(wo, N, roughness));

}

vec3 fresnel_schlick(float cosTheta, vec3 ambient_color, float metallic) {

    vec3 F0 = mix(vec3(0.04), ambient_color, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;

}

vec3 F_PBRT(vec3 wi, vec3 wh) {

    //assuming dielectrics
    float cosThetaI = clamp(dot(wi,wh),-1.0f,1.0f);
    float etaI = 1.00029; // air at sea level
    float etaT = 1.544; // water 20 Degrees

    //<< Potentially swap indices of refraction >>
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        float aux = etaI;
        etaI = etaT;
        etaT = aux;
        cosThetaI = abs(cosThetaI);
    }

    //<< Compute cosThetaT using Snell’s law >>
    float sinThetaI = sqrt(max(0.f, 1.f - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;
    //<< Handle total internal reflection >>
    if (sinThetaT >= 1)
        return vec3(1);

    float cosThetaT = sqrt(max(0.f, 1.f - sinThetaT * sinThetaT));

    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
        ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
        ((etaI * cosThetaI) + (etaT * cosThetaT));

    return vec3((Rparl * Rparl + Rperp * Rperp) / 2.f);

}

float RoughnessToAlpha_PBRT(float roughness) {

    roughness = max(roughness, 1e-3);
    float x = log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
        0.000640711f * x * x * x * x;

}

// Normal Distribution function ---------------------
float D_GGX_PBRT(vec3 wh, vec3 N, float roughness) {

    float tan2Theta = Tan2Theta(wh, N);
    // catch infinity 
    float infinity = 1.0 / 0.0;
    if (tan2Theta == infinity) return 0.;
    float alphax = RoughnessToAlpha_PBRT(roughness);
    float alphay = RoughnessToAlpha_PBRT(roughness);
    const float cos4Theta = Cos2Theta(wh, N) * Cos2Theta(wh, N);
    float e = (Cos2Phi(wh, N) / (alphax * alphax) + Sin2Phi(wh, N) / (alphay * alphay)) * tan2Theta;
    return 1.f / (PI * alphax * alphay * cos4Theta * (1.f + e) * (1.f + e));

}

float Lambda_PBRT(vec3 w, vec3 N, float roughness) {

    float absTanTheta = abs(TanTheta(w, N));
    // catch infinity
    float infinity = 1.0 / 0.0;
    if (absTanTheta == infinity) return 0.;
    float alphax = RoughnessToAlpha_PBRT(roughness);
    float alphay = RoughnessToAlpha_PBRT(roughness);
    float alpha = sqrt(Cos2Phi(w, N) * alphax * alphax +
        Sin2Phi(w, N) * alphay * alphay);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1.f + sqrt(1.f + alpha2Tan2Theta)) / 2.f;

}

// Geometric Shadowing function -----------------------
float G_GGX_PBRT(vec3 wi, vec3 wo, vec3 N, float roughness) {

    return 1.f / (1.f + Lambda_PBRT(wo, N, roughness) + Lambda_PBRT(wi, N, roughness));

}

float D_GGX_EPIC_GAMES(vec3 wh, vec3 N, float roughness)
{
    float dotNH = clamp(dot(normalize(wh), normalize(N)),0.0f, 1.0);
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

float G_GGX_EPIC_GAMES(vec3 wi, vec3 wo, vec3 N, float roughness)
{
    float dotNL = clamp(dot(normalize(wo), normalize(N)),0.0,1.0);
    float dotNV = clamp(dot(normalize(wi), normalize(N)), 0.0, 1.0);
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

vec3 F_EPIC_GAMES(vec3 wi, vec3 wh, vec3 ambient_color, float metallic) {

    float cosTheta = clamp(dot(wi, wh), 0.0f, 1.0f);
    vec3 F0 = mix(vec3(0.04), ambient_color, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(2, (- 5.55473 * cosTheta - 6.98316) * cosTheta);
    return F;

}

// mode :
// [0] --> EPIC GAMES (https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
// [1] --> PBR BOOK (https://pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models)
// [2] --> DISNEYS PRINCIPLED (https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf)
vec3 evaluateCookTorrenceBRDF(vec3 ambient, vec3 N, vec3 L, vec3 V, float roughness, int mode) {

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
    switch (mode) {
    case 0: D = D_GGX_EPIC_GAMES(wh, N, roughness);
            G = G_GGX_EPIC_GAMES(wi, wo, N, roughness);
            F = F_EPIC_GAMES(wi, wh, ambient, .9f);
            break;
    case 1: D = D_GGX_PBRT(wh, N, roughness);
            G = G_GGX_PBRT(wi, wo, N, roughness);
            F = F_PBRT(wi, wh);
            break;
    case 2: D = D_Disney(wh, N, roughness);
            G = G_Disney(wi, wo, N, roughness);
            F = fresnel_schlick(CosTheta(wh, wi), ambient, 0.1);
        break;
    }

    return vec3((D * G * F) / (4.f * cosThetaI * cosThetaO));

}