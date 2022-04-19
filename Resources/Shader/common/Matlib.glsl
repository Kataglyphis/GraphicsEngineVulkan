// some common math functions :)

const float PI = 3.14159265359;

float CosTheta(const vec3 w, vec3 N) { 
    return dot(w,N); 
}

float Cos2Theta(const vec3 w, vec3 N) { 
    float cosTheta = CosTheta(w,N);
    return cosTheta * cosTheta; 
}

float AbsCosTheta(const vec3 w, vec3 N) { 
    return abs(dot(w,N)); 
}

float Sin2Theta(const vec3 w, vec3 N) {
    return max(0, 1 - Cos2Theta(w,N));
}

float SinTheta(const vec3 w, vec3 N) {
    return sqrt(Sin2Theta(w,N));
}

float TanTheta(const vec3 w, vec3 N) {
    return SinTheta(w,N) / CosTheta(w,N);
}

float Tan2Theta(const vec3 w, vec3 N) {
    return Sin2Theta(w,N) / Cos2Theta(w,N);
}

float CosPhi(const vec3 w, vec3 N) {
    float sinTheta = SinTheta(w, N);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
}
float SinPhi(const vec3 w, vec3 N) {
    float sinTheta = SinTheta(w, N);
    return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1, 1);
}

float Cos2Phi(const vec3 w, vec3 N) {
    return CosPhi(w,N) * CosPhi(w,N);
}
float Sin2Phi(const vec3 w, vec3 N) {
    return SinPhi(w, N) * SinPhi(w,N);
}