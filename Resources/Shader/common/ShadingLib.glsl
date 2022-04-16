//all functions for lighting with microfacet model

const float PI = 3.14159265359f;

vec3 fresnel_schlick(float cos_theta) {

    //vec3 f0 = vec3(0.04f);
    //f0 = mix(f0, albedo, materials[material_id].metallic);

    float IOR = 1.0f; // air

    float n1 = 1.0f;
    float n2 = IOR;

    vec3 f0 = vec3((n1 - n2) / (n1 + n2));
    f0 = f0 * f0;

    return f0 + ((vec3(1.0f) - f0) * pow(max(0.0f, 1.0 - cos_theta), 5.0));

}

float chi_GGX(float v) {

    return v > 0.0f ? 1.0f : 0.0f;

}

// Normal Distribution function ---------------------
float D_GGX(vec3 N, vec3 H, float roughness) {

    float n_dot_h = clamp(dot(N, H),0.0,1.0);
    float n_dot_h_2 = n_dot_h * n_dot_h;
    float roughness_2 = roughness * roughness;
    float roughness_4 = roughness_2 * roughness_2;
    float distributor = (n_dot_h_2 * (roughness_4 - 1.f)) + 1.f;//+ ((1.f /n_dot_h_2) - 1.f)

    return  (chi_GGX(n_dot_h) * roughness_4) / (PI * distributor * distributor);

}

// Geometric Shadowing function -----------------------
float G_GGX_P(vec3 V, vec3 N, vec3 H, float roughness) {

    float v_dot_h = clamp(dot(V, H),0.0,1.0);
    float v_dot_n = clamp(dot(V, N),0.0,1.0);

    float v_dot_h_2 = v_dot_h * v_dot_h;

    //catch divide by zero case
    if (chi_GGX(v_dot_n) == 0.0f) return 0.0f;

    float chi = chi_GGX(v_dot_h) / chi_GGX(v_dot_n);
    //already normed
    float tan_2 = (1.f / v_dot_h_2) - 1.f;

    return max((chi * 2.f) / (1.f + sqrt(1.f + (roughness * roughness * tan_2))), 1.0f);
}

vec3 CookTorrenceBRDF(vec3 albedo, vec3 N, vec3 H, vec3 L, vec3 V, float roughness, vec3 light_color, float light_ambient_intensity) {

    if (dot(N,L) < 0) return vec3(0.0);
    vec3 F = fresnel_schlick(dot(H, V));
    
    //cook-torrance brdf  
    float D = D_GGX(N, H, roughness);
    float G = G_GGX_P(V, N, H, roughness) * G_GGX_P(L, N, H, roughness);

    //calc cook torrence brdf 
    vec3 diffuse = albedo / PI;
    vec3 specular = (D * G * F) / max(4.0f * max(0.0f, dot(V, N)) * max(dot(N, L), 0.0f), 0.001f);

    vec3 radiance = light_color * light_ambient_intensity;

    vec3 k_s = F;
    vec3 k_d = (vec3(1.0f) - k_s);

    return (diffuse * k_d + specular * k_s) * max(0.0f, dot(N, L)) * radiance;

}