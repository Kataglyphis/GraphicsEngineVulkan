// microfacet model developed by Torrance-Sparrow
#ifndef MICROFACET
#define MICROFACET
vec3 evaluateCookTorrenceSpecularBRDF(float D, float G, vec3 F, float cosThetaI, float cosThetaO) {

    return vec3((D * G * F) / (4.f * cosThetaI * cosThetaO));

}
#endif