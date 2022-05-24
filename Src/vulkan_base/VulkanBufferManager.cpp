#include "VulkanBufferManager.h"

VulkanBufferManager::VulkanBufferManager()
{
}

void VulkanBufferManager::copyBuffer(	VkDevice device, 
										VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
										VulkanBuffer src_buffer, VulkanBuffer dst_buffer, 
										VkDeviceSize buffer_size)
{
	// create buffer
	VkCommandBuffer command_buffer = begin_command_buffer(device, transfer_command_pool);

	// region of data to copy from and to
	VkBufferCopy buffer_copy_region{};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = buffer_size;

	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(command_buffer, src_buffer.getBuffer(), dst_buffer.getBuffer(), 1, &buffer_copy_region);

	end_and_submit_command_buffer(device, transfer_command_pool, transfer_queue, command_buffer);
}

VulkanBufferManager::~VulkanBufferManager()
{
}
