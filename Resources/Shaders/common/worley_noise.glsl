#include "/host_device_shared.hpp"

// worley noise implementation;
// inspired by Sebastian Lague: https://www.youtube.com/watch?v=4QOcCGI6xOU&t=104s

// in 3D-space there are 26 cells surrounding each cell
const int   NUM_ADJACENT_CELLS      = 26;

const vec3 offsets[NUM_ADJACENT_CELLS + 1] =
{
    // centre
    vec3(0,0,0),
    // front face
    vec3(0,0,1),
    vec3(-1,1,1),
    vec3(-1,0,1),
    vec3(-1,-1,1),
    vec3(0,1,1),
    vec3(0,-1,1),
    vec3(1,1,1),
    vec3(1,0,1),
    vec3(1,-1,1),
    // back face
    vec3(0,0,-1),
    vec3(-1,1,-1),
    vec3(-1,0,-1),
    vec3(-1,-1,-1),
    vec3(0,1,-1),
    vec3(0,-1,-1),
    vec3(1,1,-1),
    vec3(1,0,-1),
    vec3(1,-1,-1),
    // ring around centre
    vec3(-1,1,0),
    vec3(-1,0,0),
    vec3(-1,-1,0),
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(1,1,0),
    vec3(1,0,0),
    vec3(1,-1,0)
};

float remap(float value, float start1, float stop1, float start2, float stop2) {

    return  start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));

}
/**

    sample_id       : vec3      ==> current shader invocation id
    num_cells       : int       ==> number of cells per dimension
    cell_positions  : sampler3D ==> stores random position per cell
    resolution      : float     ==> resolution of texture to store
 
*/
float worley(   vec3        sample_id,
                int         num_cells,
                sampler3D   cell_positions,
                float       resolution) {

    // 1.step: scale current sample id according to frequency level
    vec3 sample_pos = (sample_id / float(resolution)) * float(num_cells);
    vec3 cell_id = floor(sample_pos);

    float min_dist = 1.f;

    for (int i = 0; i < NUM_ADJACENT_CELLS + 1; i++) {

        vec3 adjacent_id = floor(cell_id + offsets[i]);

        bool out_of_scope = adjacent_id.x == -1 || int(adjacent_id.x) == num_cells ||
                            adjacent_id.y == -1 || int(adjacent_id.y) == num_cells ||
                            adjacent_id.z == -1 || int(adjacent_id.z) == num_cells;


        // at this point we are tilling the image
        if (out_of_scope) {
            
            // excatly this is tilling: make sure your current position comes out at the opposite direction
            vec3 wrapped_id = mod(adjacent_id + vec3(num_cells), vec3(num_cells));
            vec3 wrapped_position = texture(cell_positions, (wrapped_id) / float(num_cells)).xyz;
            for (int m = 0; m < NUM_ADJACENT_CELLS; m++) {

                //after wrapping one has to check once again for lowest difference     
                vec3 difference = sample_pos - (wrapped_position + (offsets[m] * num_cells));
                // our metric here is L2
                min_dist = min(min_dist, dot(difference, difference));

            }

        } else {

            vec3 difference = sample_pos - texture(cell_positions, adjacent_id / float(num_cells)).xyz;
            min_dist = min(min_dist, dot(difference, difference));

        }

    }

    return sqrt(min_dist);

}

// here we want to layer fixed 3 frequencies according to fbm
const int   NUM_WORLEY_FREQUENCIES = 3;

// really cool article: https://iquilezles.org/articles/fbm/
float fbm_worley(float worley_noise[NUM_WORLEY_FREQUENCIES], float H)
{

    float gain = exp2(-H);
    float amplitude = .6f;
    float result = 0.0f;

    for (int i = 0; i < NUM_WORLEY_FREQUENCIES; i++)
    {
        result += amplitude * worley_noise[i];
        amplitude *= gain;
    }

    return result;

}