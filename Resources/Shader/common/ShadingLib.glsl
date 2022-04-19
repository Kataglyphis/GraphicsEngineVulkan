//all functions for lighting with microfacet model
#include "Matlib.glsl"

vec3 fresnel(float cos_theta) {

    //assuming dielectrics

    float etaI = 1.00029; // air at sea level
    float etaT = 1.333; // water 20 Degrees

    cosThetaI = clamp(cos_theta, -1, 1);
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    //Compute cosThetaT using Snell’s law
    float sinThetaI = sqrt(max((float)0,
            1 - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;
    // Handle total internal reflection
    float cosThetaT = sqrt(max((float)0,
            1 - sinThetaT * sinThetaT));

    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
        ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
        ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;

    //float IOR = 1.0f; // air

    //float n1 = 1.0f;
    //float n2 = IOR;

    //vec3 f0 = vec3((n1 - n2) / (n1 + n2));
    //f0 = f0 * f0;

    //return f0 + ((vec3(1.0f) - f0) * pow(max(0.0f, 1.0 - cos_theta), 5.0));

}

float chi_GGX(float v) {

    return v > 0.0f ? 1.0f : 0.0f;

}

// Normal Distribution function ---------------------
float D_GGX(vec3 wh, vec3 N) {

    float tan2Theta = Tan2Theta(wh,N);
    if (std::isinf(tan2Theta)) return 0.;
    const float cos4Theta = Cos2Theta(wh,N) * Cos2Theta(wh,N);
    float e = (Cos2Phi(wh,N) / (alphax * alphax) +
        Sin2Phi(wh) / (alphay * alphay)) * tan2Theta;
    return 1 / (Pi * alphax * alphay * cos4Theta * (1 + e) * (1 + e));

}

float Lamda(vec3 w) {

    float absTanTheta = abs(TanTheta(w));
    if (std::isinf(absTanTheta)) return 0.;
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;

}

// Geometric Shadowing function -----------------------
float G_GGX(vec3 wi, vec3 wo) {

    return 1 / (1 + Lambda(wo) + Lambda(wi));

    //float v_dot_h = clamp(dot(V, H),0.0,1.0);
    //float v_dot_n = clamp(dot(V, N),0.0,1.0);

    //float v_dot_h_2 = v_dot_h * v_dot_h;

    ////catch divide by zero case
    //if (chi_GGX(v_dot_n) == 0.0f) return 0.0f;

    //float chi = chi_GGX(v_dot_h) / chi_GGX(v_dot_n);
    ////already normed
    //float tan_2 = (1.f / v_dot_h_2) - 1.f;

    //return max((chi * 2.f) / (1.f + sqrt(1.f + (roughness * roughness * tan_2))), 1.0f);
}

vec3 CookTorrenceBRDF(vec3 albedo, vec3 N, vec3 H, vec3 L, vec3 V, float roughness, vec3 light_color, float light_ambient_intensity) {

    vec3 wo = normalize(-L);
    vec3 wi = normalize(-V);
    float cosThetaO = AbsCosTheta(wo,N); 
    float cosThetaI = AbsCosTheta(wi,N);
    vec3 wh = wi + wo;
    if (cosThetaI == 0 || cosThetaO == 0) return vec3(0);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return vec3(0);
    wh = Normalize(wh);
    vec3 F = fresnel(dot(wi, wh));

    return roughness * D_GGX(wh,N) * G_GGX(wo, wi) * F /
            (4.f * cosThetaI * cosThetaO);
    //if (dot(N,L) < 0) return vec3(0.0);
    //vec3 F = fresnel_schlick(dot(H, V));
    //
    ////cook-torrance brdf  
    //float D = D_GGX(N, H, roughness);
    //float G = G_GGX_P(V, N, H, roughness) * G_GGX_P(L, N, H, roughness);

    ////calc cook torrence brdf 
    //vec3 diffuse = albedo / PI;
    //vec3 specular = (D * G * F) / max(4.0f * max(0.0f, dot(V, N)) * max(dot(N, L), 0.0f), 0.001f);

    //vec3 radiance = light_color * light_ambient_intensity;

    //vec3 k_s = F;
    //vec3 k_d = (vec3(1.0f) - k_s);

    //return (diffuse * k_d + specular * k_s) * max(0.0f, dot(N, L)) * radiance;

}