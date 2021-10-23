#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//#include "../../ExternalLib/GLM/glm/glm.hpp"

// vertex data representation (layout)
struct Vertex {

	glm::vec3 pos;
	alignas(16) glm::vec2 texture_coords;
	glm::vec3 normal;

};

struct ObjectDescription {

	int texture_offset;
	uint64_t vertex_address;
	uint64_t index_address;

};

struct UboViewProjection {

	glm::mat4 projection;
	glm::mat4 view;

};

struct UboDirections {

	glm::vec3 light_dir;
	glm::vec3 view_dir;

};

// Push constant structure for the raster
struct PushConstantRaster
{

	glm::mat4  model;  // matrix of the instance

};

// Push constant structure for the ray tracer
struct PushConstantRay
{
	glm::vec4  clearColor;
};

struct RaytracingPushConstants {

	glm::vec4 clear_color;

};

// uniform
struct UboRaytracing {

	glm::vec4 camera_position;
	glm::vec4 right;
	glm::vec4 up;
	glm::vec4 forward;

};

enum SceneBindings {

	GLOBALS_BINDING = 0,
	OBJECT_DESCRIPTION_BINDING = 1,
	TEXTURES_BINDING = 2

};

enum RaytracingBindings {

	TLAS_BINDING = 0,
	OUT_IMAGE = 1

};