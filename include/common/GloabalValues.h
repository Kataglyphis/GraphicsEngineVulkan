#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <vulkan/vulkan.h>

const int MAX_FRAME_DRAWS = 3;
const int MAX_OBJECTS = 40;
const int NUM_RAYTRACING_DESCRIPTOR_SET_LAYOUTS = 2;
const int MAX_TEXTURE_COUNT = 24;

// use the standard validation layers from the SDK for error checking
const std::vector<const char*> validationLayers = {
					"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

enum SHADER_COMPILATION_FLAG {
	RASTERIZATION,
	RAYTRACING,
	POST
};

// DEVICE EXTENSIONS FOR RAYTRACING
const std::vector<const char*> device_extensions_for_raytracing = {

	// raytracing related extensions 
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	// required from VK_KHR_acceleration_structure
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	// required for pipeline
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	// required by VK_KHR_spirv_1_4
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	//required for pipeline library
	VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, 



};

struct SwapChainDetails {

	VkSurfaceCapabilitiesKHR surface_capabilities;				// surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;					// surface image formats, e.g. RGBA and size of each color
	std::vector<VkPresentModeKHR> presentation_mode;  // how images should be presented to screen

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

struct UboViewProjection {

	glm::mat4 projection;
	glm::mat4 view;

};

struct UboDirections {

	glm::vec4 light_dir;
	glm::vec4 view_dir;
	glm::vec4 cam_pos;

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

