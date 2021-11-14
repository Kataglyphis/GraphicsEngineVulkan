#pragma once

#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <vector>

const int MAX_FRAME_DRAWS = 3;
const int MAX_OBJECTS = 20;
const int NUM_RAYTRACING_DESCRIPTOR_SET_LAYOUTS = 2;

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
	RAYTRACING
};

const std::vector<const char*> device_extensions = {

	VK_KHR_SWAPCHAIN_EXTENSION_NAME

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

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices {

	int graphics_family = -1;																// location of graphics family
	int presentation_family = -1;														// location of presentation queue family
	int compute_family = -1;																// location of compute queue family

	//check if queue families are valid 
	bool is_valid() {

		return graphics_family >= 0 && presentation_family >= 0 && compute_family >= 0;

	}

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

// vertex data representation (layout)
struct Vertex {

	glm::vec3 pos;
	alignas(16) glm::vec2 texture_coords;
	glm::vec3 normal;

};

struct ObjectDescription {
	
	uint64_t texture_id;
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

struct PushConstantRaytracing {

	glm::vec4 clear_color;

};

struct BLAS {

	VkAccelerationStructureKHR accel = VK_NULL_HANDLE;
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

