#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "Utilities.h"
#include "VulkanBuffer.h"

class VulkanBufferManager
{
public:

	VulkanBufferManager();

	template<typename T>
	void createBufferAndUploadVectorOnDevice(	VulkanDevice* device, 
												VulkanBuffer& vulkanBuffer,
												VkBufferUsageFlags dstBufferUsageFlags,
												VkMemoryPropertyFlags dstBufferMemoryPropertyFlags,
												std::vector<T> data);

	~VulkanBufferManager();

private:

};

template<typename T>
inline void VulkanBufferManager::createBufferAndUploadVectorOnDevice(	VulkanDevice* device, 
																		VulkanBuffer& vulkanBuffer,
																		VkBufferUsageFlags dstBufferUsageFlags,
																		VkMemoryPropertyFlags dstBufferMemoryPropertyFlags,
																		std::vector<T> data)
{
	VkDeviceSize bufferSize = sizeof(T) * data.size();

	// temporary buffer to "stage" vertex data before transfering to GPU
	VulkanBuffer stagingBuffer;

	// create buffer and allocate memory to it
	stagingBuffer.create(	device.get(), bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map memory to vertex buffer
	// 1.) create pointer to a point in normal memory
	void* data;
	// 2.) map the vertex buffer memory to that point
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getBufferMemory(), 0, bufferSize, 0, &data);
	// 3.) copy memory from vertices vector to the point
	memcpy(data, data.data(), (size_t)bufferSize);
	// 4.) unmap the vertex buffer memory
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getBufferMemory());

	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	vulkanBuffer.create(device.get(),
						bufferSize,
						dstBufferUsageFlags,
						dstBufferMemoryPropertyFlags);

	// copy staging buffer to vertex buffer on GPU
	/*copy_buffer(device->getLogicalDevice(), device->getComputeQueue(), compute_command_pool,
				stagingBuffer.getBuffer(), vulkanBuffer.getBuffer(), bufferSize);*/
}
