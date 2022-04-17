#pragma once
#include <vulkan/vulkan.h>

class BottomLevelAccelerationStructure
{

public:

	VkAccelerationStructureKHR accel;
	VkBuffer buffer;
	VkDeviceMemory memory;

private:


};

