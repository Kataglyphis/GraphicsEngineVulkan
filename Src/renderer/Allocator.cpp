#include "Allocator.h"

Allocator::Allocator()
{
}

Allocator::Allocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance)
{

	// see here: https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;

	if (vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator) != VK_SUCCESS) {

		throw std::runtime_error("Failed to create vma allocator!");

	};

}

Allocator::~Allocator()
{
	vmaDestroyAllocator(vmaAllocator);
}
