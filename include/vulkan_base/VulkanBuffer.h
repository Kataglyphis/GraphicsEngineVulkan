#pragma once
#include <vulkan/vulkan.h>

#include "VulkanDevice.h"

class VulkanBuffer
{
public:

	VulkanBuffer();

	void			create(	VulkanDevice* vulkanDevice,
							VkDeviceSize buffer_size, 
							VkBufferUsageFlags buffer_usage_flags,
							VkMemoryPropertyFlags buffer_propertiy_flags);

	void			cleanUp();

	VkBuffer&		getBuffer() { return buffer; };
	VkDeviceMemory& getBufferMemory() { return bufferMemory; };

	~VulkanBuffer();

private:

	VulkanDevice*	device;

	VkBuffer		buffer;
	VkDeviceMemory	bufferMemory;

	bool created	= false;
};

