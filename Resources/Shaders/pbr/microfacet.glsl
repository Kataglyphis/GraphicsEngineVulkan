#ifndef MICROFACET
#define MICROFACET

// microfacet model developed by Torrance-Sparrow
vec3 evaluateCookTorrenceSpecularBRDF(float D, float G, vec3 F, float cosThetaI, float cosThetaO) {

    return vec3((D * G * F) / (4.f * cosThetaI * cosThetaO));

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (86)
float GGX_SMITH_LAMBDA(vec3 v, vec3 N, float roughness, float alphax, float alphay) {

    float alphax2 = alphax * alphax;
    float alphay2 = alphay * alphay;
    //equation (80)
    float alpha_0 = sqrt(Cos2Phi(v,N) * alphax2 + Sin2Phi(v, N) * alphay2);
    float a = 1.f / (alpha_0 * TanTheta(v,N));
    
    return (-1.f + sqrt(1.f + (1.f / pow(a,2)))) / 2.f;

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (43)
float G1_GGX_SMITH(vec3 v, vec3 N, float roughness, float alphax, float alphay) {

    return 1.f / (1.f + GGX_SMITH_LAMBDA(v,N, roughness, alphax, alphay));

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (55)
float G2_GGX_SMITH(vec3 wi, vec3 wo, vec3 N, float roughness, float alphax, float alphay) {

    if (dot(wi, N) <= 0.f || dot(wo, N) <= 0.f) return 0.f;
    return G1_GGX_SMITH(wi , N, roughness, alphax, alphay) * G1_GGX_SMITH(wo, N, roughness, alphax, alphay);

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (72)
float GGX_SMITH_LAMBDA_ISOTROPIC(vec3 v, vec3 N, float roughness) {

    float a = 1.f / (roughness * TanTheta(v, N));

    return (-1.f + sqrt(1.f + (1.f / pow(a, 2)))) / 2.f;

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (43)
float G1_GGX_SMITH_ISOTROPIC(vec3 wi, vec3 N, float roughness) {

    return 1.f / (1.f + GGX_SMITH_LAMBDA_ISOTROPIC(wi, N, roughness));

}

// ref: https://hal.inria.fr/hal-01024289/
// Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs
// by Eric Heitz
// equation (55)
float G2_GGX_SMITH_ISOTROPIC(vec3 wi, vec3 wo, vec3 N, float roughness) {

    if (dot(wi, N) <= 0.f || dot(wo, N) <= 0.f) return 0.f;
    return G1_GGX_SMITH_ISOTROPIC(wi, N, roughness) * G1_GGX_SMITH_ISOTROPIC(wo, N, roughness);

}

#endif