#include "Matlib.glsl"

vec3 fresnel_schlick(float cosTheta, vec3 ambient_color, float metallic) {

    vec3 F0 = mix(vec3(0.04), ambient_color, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;

}

vec3 F_PBRT(vec3 wi, vec3 wh) {

    //assuming dielectrics
    float cosThetaI = clamp(dot(wi,wh),0.0f,1.0f);
    float etaI = 1.00029; // air at sea level
    float etaT = 1.544; // water 20 Degrees

    cosThetaI = clamp(cosThetaI, -1, 1);
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
    vec3 F = F0 + (1.0 - F0) * pow(2, -5.55473 * cosTheta - 6.98316 * cosTheta);
    return F;

}

// mode :
// [0] --> EPIC GAMES (https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
// [1] --> PBR BOOK (https://pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models)
vec3 evaluateCookTorrenceBRDF(vec3 ambient, vec3 N, vec3 L, vec3 V, float roughness, int mode) {

    vec3 wo = normalize(L);
    vec3 wi = normalize(V);

    if (mode == 1) {
        // the pbr book assumes the view vector to point away from the surface
        wi *= -1.f;
    }

    float cosThetaO = AbsCosTheta(wo, N);
    float cosThetaI = AbsCosTheta(wi, N);
    vec3 wh = wi + wo;
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0);
    //if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0);
    wh = normalize(wh);
    //float F = fresnel_PBRT(dot(wi, wh));
    //vec3 F = fresnel_schlick(dot(wi, wh), ambient, 0.001);
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
    }

    return vec3((D * G * F) / (4.f * cosThetaI * cosThetaO));

}