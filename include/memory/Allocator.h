#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <stdexcept>

class Allocator
{

public:

	Allocator();
	Allocator(	const VkDevice& device, 
				const VkPhysicalDevice& physicalDevice, 
				const VkInstance& instance);

	~Allocator();

private:

	VmaAllocator vmaAllocator;

};

