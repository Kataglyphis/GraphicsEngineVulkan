#pragma once

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector>
#include <vulkan/vulkan.h>

#include "host_device_shared_vars.h"

static VkFormat choose_supported_format(VkPhysicalDevice physical_device,
										const std::vector<VkFormat>&formats, 
										VkImageTiling tiling, 
										VkFormatFeatureFlags feature_flags)
{

	// loop through options and find compatible one
	for (VkFormat format : formats) {

		// get properties for give format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

		// depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}

	}

	throw std::runtime_error("Failed to find supported format!");

}

static VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool) {

	// command buffer to hold transfer commands
	VkCommandBuffer command_buffer;

	// command buffer details
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = 1;

	// allocate command buffer from pool
	vkAllocateCommandBuffers(device, &alloc_info, & command_buffer);

	// infromation to begin the command buffer record
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// we are only using the command buffer once, so set up for one time submit
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	// begin recording transfer commands
	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;

}

static void end_and_submit_command_buffer(	VkDevice device, VkCommandPool command_pool, 
											VkQueue queue, VkCommandBuffer& command_buffer) {

	// end commands
	VkResult result = vkEndCommandBuffer(command_buffer);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to end command buffer!");

	}

	// queue submission information 
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	// submit transfer command to transfer queue and wait until it finishes
	result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to submit to queue!");

	}

	result = vkQueueWaitIdle(queue);

	if (result != VK_SUCCESS) {

		printf("%i", result);
		throw std::runtime_error("Failed to wait Idle!");

	}

	// free temporary command buffer back to pool
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);

}