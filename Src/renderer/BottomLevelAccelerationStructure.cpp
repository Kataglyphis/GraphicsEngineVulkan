#include "BottomLevelAccelerationStructure.h"

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure()
{
}

void BottomLevelAccelerationStructure::cleanUp()
{
	//PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR =
	//	(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device->getLogicalDevice(), "vkDestroyAccelerationStructureKHR");

	//pvkDestroyAccelerationStructureKHR(device->getLogicalDevice(), vulkanAS, nullptr);

}

BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
}
