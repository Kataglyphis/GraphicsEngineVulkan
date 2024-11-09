#ifndef MATERIAL
#define MATERIAL

struct Material {

	vec3    ambient;
	vec3    diffuse;
	vec3    specular;
	vec3    transmittance;
	vec3    emission;
	float   shininess;
	float   ior;		// index of refraction
	float   dissolve;	// 1 == opaque; 0 == fully transparent

	int     illum;
	int     textureID;

};

#endif