#include "TopLevelAccelerationStructure.h"

TopLevelAccelerationStructure::TopLevelAccelerationStructure()
{
}

void TopLevelAccelerationStructure::cleanUp(VulkanDevice* device)
{
	PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device->getLogicalDevice(), "vkDestroyAccelerationStructureKHR");

	pvkDestroyAccelerationStructureKHR(device->getLogicalDevice(), vulkanAS, nullptr);

	vulkanBuffer.cleanUp();

}

TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
}
