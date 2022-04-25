#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <stdexcept>

class Allocator
{

public:

	Allocator();
	Allocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance);

	~Allocator();

private:

	VmaAllocator vmaAllocator;

};

