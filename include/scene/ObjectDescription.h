// this little "hack" is needed for using it on the
// CPU side as well for the GPU side :)
// inspired by the NVDIDIA tutorial:
// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/

#ifdef __cplusplus
#pragma once
#include <vulkan/vulkan.h>
#endif

struct ObjectDescription {

	uint64_t vertex_address;
	uint64_t index_address;
	uint64_t material_index_address;
	uint64_t material_address;

};