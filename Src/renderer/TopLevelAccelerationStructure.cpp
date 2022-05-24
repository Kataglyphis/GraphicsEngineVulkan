#include "TopLevelAccelerationStructure.h"

TopLevelAccelerationStructure::TopLevelAccelerationStructure()
{
}

void TopLevelAccelerationStructure::cleanUp()
{
	/*PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device->getLogicalDevice(), "vkDestroyAccelerationStructureKHR");

	pvkDestroyAccelerationStructureKHR(device->getLogicalDevice(), vulkanAS, nullptr);*/

}

TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
}
