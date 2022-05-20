#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <vulkan/vulkan.h>

const int MAX_FRAME_DRAWS = 3;
const int MAX_OBJECTS = 40;
const int NUM_RAYTRACING_DESCRIPTOR_SET_LAYOUTS = 2;
const int MAX_TEXTURE_COUNT = 24;

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct SwapChainDetails {

	VkSurfaceCapabilitiesKHR surface_capabilities;		// surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;			// surface image formats, e.g. RGBA and size of each color
	std::vector<VkPresentModeKHR> presentation_mode;	// how images should be presented to screen

};

struct SwapChainImage {

	VkImage image;
	VkImageView image_view;

};

struct ObjectDescription {
	
	uint64_t vertex_address;
	uint64_t index_address;
	uint64_t material_index_address;
	uint64_t material_address;

};

// Push constant structure for the raster
struct PushConstantRaster
{

	glm::mat4 model;  // matrix of the instance

};

struct PushConstantRaytracing {

	glm::vec4 clear_color;

};

struct PushConstantPost {

	float aspect_ratio;

};

struct BLAS {

	VkAccelerationStructureKHR accel;
	VkBuffer buffer;
	VkDeviceMemory memory;

};

struct BuildAccelerationStructure {

	VkAccelerationStructureBuildGeometryInfoKHR build_info;
	VkAccelerationStructureBuildSizesInfoKHR size_info;
	const VkAccelerationStructureBuildRangeInfoKHR* range_info;
	BLAS single_blas;

};

struct BlasInput {

	std::vector<VkAccelerationStructureGeometryKHR> as_geometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> as_build_offset_info;

};

struct TLAS {

	VkAccelerationStructureKHR top_level_acceleration_structure;
	VkBuffer top_level_acceleration_structure_buffer;
	VkDeviceMemory top_level_acceleration_structure_buffer_memory;

};

struct RayTracingImage {

	VkImageView raytracing_image_view;
	VkImage raytracing_image;
	VkDeviceMemory ray_tracing_image_memory;

};

struct OffScreenRenderImage {

	VkImageView image_view;
	VkImage image;
	VkDeviceMemory image_memory;

};

struct OffscreenTexture {

	VkImageView image_view;
	VkImage image;
	VkDeviceMemory image_memory;

};

