#pragma once

#include <vulkan/vulkan.h>

class TopLevelAccelerationStructure
{
public:

	VkAccelerationStructureKHR top_level_acceleration_structure;
	VkBuffer top_level_acceleration_structure_buffer;
	VkDeviceMemory top_level_acceleration_structure_buffer_memory;

private:


};

