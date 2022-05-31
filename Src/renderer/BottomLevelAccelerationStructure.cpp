#include "BottomLevelAccelerationStructure.h"

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure()
{
}

void BottomLevelAccelerationStructure::cleanUp(VulkanDevice* device)
{
	PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device->getLogicalDevice(), "vkDestroyAccelerationStructureKHR");

	pvkDestroyAccelerationStructureKHR(device->getLogicalDevice(), vulkanAS, nullptr);

	vulkanBuffer.cleanUp();

}

BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
}
