#pragma once
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

class BottomLevelAccelerationStructure
{

public:

	VkAccelerationStructureKHR	accel;
	VulkanBuffer				vulkanBuffer;

private:


};

