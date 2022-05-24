#pragma once

#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"

class TopLevelAccelerationStructure
{
public:

	TopLevelAccelerationStructure();

	VkAccelerationStructureKHR& getAS() { return vulkanAS; };
	VulkanBuffer&				getVulkanBuffer() { return vulkanBuffer; };

	void						create();

	void						cleanUp();

	~TopLevelAccelerationStructure();

private:

	VkAccelerationStructureKHR	vulkanAS;
	VulkanBuffer				vulkanBuffer;

};

