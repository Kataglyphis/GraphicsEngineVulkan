#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

struct BLAS {

	VkAccelerationStructureKHR	accel;
	VulkanBuffer				vulkanBuffer;

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
	VulkanBuffer vulkanBuffer;

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

