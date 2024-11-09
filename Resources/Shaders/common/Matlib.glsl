// some common math functions :)
#ifndef MATH_LIB
#define MATH_LIB

#ifndef PI_DEF
#define PI_DEF
const float PI = 3.14159265358979323846;
#endif

float CosTheta(const vec3 w, vec3 N) { return dot(w, N); }

float Cos2Theta(const vec3 w, vec3 N)
{
    float cosTheta = CosTheta(w, N);
    return cosTheta * cosTheta;
}

float AbsCosTheta(const vec3 w, vec3 N) { return abs(dot(w, N)); }

float Sin2Theta(const vec3 w, vec3 N) { return max(0.f, 1.f - Cos2Theta(w, N)); }

float SinTheta(const vec3 w, vec3 N) { return sqrt(Sin2Theta(w, N)); }

float TanTheta(const vec3 w, vec3 N) { return SinTheta(w, N) / CosTheta(w, N); }

float Tan2Theta(const vec3 w, vec3 N) { return Sin2Theta(w, N) / Cos2Theta(w, N); }

float CosPhi(const vec3 w, vec3 N)
{
    float sinTheta = SinTheta(w, N);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1.f, 1.f);
}
float SinPhi(const vec3 w, vec3 N)
{
    float sinTheta = SinTheta(w, N);
    return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1.f, 1.f);
}

float Cos2Phi(const vec3 w, vec3 N) { return CosPhi(w, N) * CosPhi(w, N); }
float Sin2Phi(const vec3 w, vec3 N) { return SinPhi(w, N) * SinPhi(w, N); }

// Calcs intersection and exit distances, normal, face and UVs
// row is the ray origin in world space
// rdw is the ray direction in world space
// txx is the world-to-box transformation
// txi is the box-to-world transformation
// ro and rd are in world space
// rad is the half-length of the box
//
// oT contains the entry and exit points
// oN is the normal in world space
// oU contains the UVs at the intersection point
// oF contains the index if the intersected face [0..5]
// source: https://iquilezles.org/articles/boxfunctions/
bool box_intersect_with_ray(in vec3 row,
  in vec3 rdw,
  in mat4 txx,
  in mat4 txi,
  in vec3 rad,
  out vec2 oT,
  out vec3 oN,
  out vec2 oU,
  out int oF)
{
    // convert from world to box space
    vec3 rd = (txx * vec4(rdw, 0.0)).xyz;
    vec3 ro = (txx * vec4(row, 1.0)).xyz;


    // ray-box intersection in box space
    vec3 m = 1.0 / rd;
    vec3 s = vec3((rd.x < 0.0) ? 1.0 : -1.0, (rd.y < 0.0) ? 1.0 : -1.0, (rd.z < 0.0) ? 1.0 : -1.0);
    vec3 t1 = m * (-ro + s * rad);
    vec3 t2 = m * (-ro - s * rad);

    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);

    if (tN > tF || tF < 0.0) return false;

    // compute normal (in world space), face and UV
    if (t1.x > t1.y && t1.x > t1.z) {
        oN = txi[0].xyz * s.x;
        oU = ro.yz + rd.yz * t1.x;
        oF = (1 + int(s.x)) / 2;
    } else if (t1.y > t1.z) {
        oN = txi[1].xyz * s.y;
        oU = ro.zx + rd.zx * t1.y;
        oF = (5 + int(s.y)) / 2;
    } else {
        oN = txi[2].xyz * s.z;
        oU = ro.xy + rd.xy * t1.z;
        oF = (9 + int(s.z)) / 2;
    }
    oT = vec2(tN, tF);

    return true;
}

#endif