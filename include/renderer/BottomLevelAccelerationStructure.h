#pragma once
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

class BottomLevelAccelerationStructure
{

public:

	BottomLevelAccelerationStructure();

	VkAccelerationStructureKHR& getAS() { return vulkanAS; };
	VulkanBuffer&				getVulkanBuffer() { return vulkanBuffer; };
	void						cleanUp(VulkanDevice* device);

	~BottomLevelAccelerationStructure();

private:

	VkAccelerationStructureKHR	vulkanAS;
	VulkanBuffer				vulkanBuffer;

};

