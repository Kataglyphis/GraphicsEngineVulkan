#pragma once
#include <vulkan/vulkan.h>

class VulkanBuffer
{
public:

	VulkanBuffer();

	~VulkanBuffer();

private:

	VkBuffer		buffer;
	VkDeviceMemory	bufferMemory;

};

