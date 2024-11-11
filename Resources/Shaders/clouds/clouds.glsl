#include "/host_device_shared.hpp"
#include "/Matlib.glsl"
#include "/directional_light.glsl"

#ifndef CLOUDS
#define CLOUDS

struct Clouds {

    int     num_march_steps;
    int     num_march_steps_to_light;

    vec3    radius;
    vec3    offset;
    mat4    model_to_world;

    float   threshold;
    float   scale;
    float   pillowness;
    float   cirrus_effect;

    bool    powder_effect;

};

float sample_density(   vec3 position, Clouds cloud, 
                        sampler3D noise_texture_1, 
                        sampler3D noise_texture_2) {

    // we are tiling a noise texture on the screen; first texture dim is 128
    // convert to [0,1]

    vec3 uvw_128        =   abs(mod(position + cloud.offset, 256.f))/ 256.f;
    vec4 noise_128      =   texture(noise_texture_1, uvw_128);

    // how you combine the different densities is up to you
    // those values following give some plausible results

    // threshold = 1- density
    float base_density = ((cloud.pillowness * 0.25f)) * max(0.0f, noise_128.r - cloud.threshold) * cloud.scale
                            +((1.f - cloud.pillowness) * 0.125f) * max(0.0f, noise_128.g - cloud.threshold) * cloud.scale
                            + ((1.f - cloud.pillowness) * 0.125f) * max(0.0f, noise_128.b - cloud.threshold) * cloud.scale;

    base_density        =   base_density * (1.f - cloud.cirrus_effect) + 
                            max(0.0f, noise_128.a - cloud.threshold) * cloud.scale * (cloud.cirrus_effect);

    vec3 uvw_32         = abs(mod(position + cloud.offset, 256.f)) / 256.f;
    vec4 noise_32       = texture(noise_texture_2, uvw_32);

    float fine_density  =    0.6f * max(0.0f, noise_32.r - cloud.threshold) * cloud.scale
                            + 0.15f * max(0.0f, noise_32.g - cloud.threshold) * cloud.scale
                            + 0.15f * max(0.0f, noise_32.b - cloud.threshold) * cloud.scale;

    return  base_density + fine_density * 0.3f;

}

// https://www.pbr-book.org/3ed-2018/Volume_Scattering/Phase_Functions
float phase_HG(float cosTheta, float g) {

    float denom = 1.f + g * g + 2.f * g * cosTheta;
    return (1.f / (4.f * PI)) * ((1.f - g * g) / (denom * sqrt(denom)));

}

float light_march(  vec3 sample_pos,
                    vec3 eye_position,
                    DirectionalLight directional_light,
                    Clouds cloud, 
                    sampler3D noise_texture_1,
                    sampler3D noise_texture_2) {

    float total_density = 0.0f;
    // no absorption for now
    float light_absorption = 1.0f;

    vec3 direction_to_light = normalize(directional_light.direction);
    vec2 oT, oU;
    vec3 oN;
    int oF;
    bool intersection = box_intersect_with_ray( eye_position, 
                                                direction_to_light, 
                                                inverse(cloud.model_to_world),
                                                cloud.model_to_world, 
                                                cloud.radius,
                                                oT, oN, oU, oF);

    for (int i = 0; i < cloud.num_march_steps_to_light; i++) {

        float step_size = (float(i) / float(cloud.num_march_steps_to_light)) * (oT.y - oT.x);
        sample_pos += direction_to_light * (step_size);

        total_density = sample_density( sample_pos,
                                        cloud,
                                        noise_texture_1,
                                        noise_texture_2);

    }

    total_density /= cloud.num_march_steps_to_light;

    float transmittance = exp(-total_density * light_absorption);

    return transmittance;

}

void calc_clouds(   mat4 projection, mat4 view, 
                    vec4 frag_pos, vec3 eye_position, 
                    Clouds cloud, float rnd_numb, 
                    sampler3D noise_texture_1,
                    sampler3D noise_texture_2, 
                    DirectionalLight directional_light,
                    float far_plane,
                    inout vec4 color) {

    vec3 ray_direction = normalize(frag_pos.xyz - eye_position);

    // oT contains the entry and exit points
    // oU contains the UVs at the intersection point
    // oF contains the index if the intersected face [0..5]
    vec2 oT, oU;
    vec3 oN;
    int oF;
    bool intersection = box_intersect_with_ray( eye_position, ray_direction, 
                                                inverse(cloud.model_to_world),
                                                cloud.model_to_world, cloud.radius, 
                                                oT, oN, oU, oF);

    float light_energy = 0.0f;
    float transmittance = 1.f;

    if (intersection) {

        for (int i = 0; i < cloud.num_march_steps; i++) {

            float step_size = (float(i) / float(cloud.num_march_steps)) * (oT.y - oT.x);
            vec3 sample_pos = eye_position + ray_direction * (oT.x + step_size);

            float density_of_sample = sample_density(   sample_pos, cloud, 
                                                        noise_texture_1, 
                                                        noise_texture_2);

            // how much light energy arrives at current ray marching step ? 
            float light_transmittance = light_march(sample_pos, 
                                                    eye_position,
                                                    directional_light, 
                                                    cloud, 
                                                    noise_texture_1,
                                                    noise_texture_2);

            // integration step
            light_energy += density_of_sample   * 
                            step_size           * 
                            transmittance       * 
                            light_transmittance *
                            phase_HG(dot(normalize(ray_direction), normalize(directional_light.direction)), 0.5f);

            // beers law homogeneous media; as we step forward in medium update the 
            // total transmittance of energy 
            transmittance *= exp(-density_of_sample * step_size);

            // we don't care about negligible energy
            if (transmittance < 0.01f) {
                break;
            }

            if (cloud.powder_effect) {

                float powderness = 1.0f - exp(-(density_of_sample * step_size) / 2.f);
                transmittance += powderness;

            }

        }

        vec3 cloud_color        = light_energy * directional_light.base.color * directional_light.base.radiance;
        color                   = color * (transmittance) + vec4(cloud_color, 1.0f);

    }

}

#endif