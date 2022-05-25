#pragma once

#include <fstream>
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

static uint32_t find_memory_type_index(	VkPhysicalDevice physical_device, uint32_t allowed_types, 
										VkMemoryPropertyFlags properties)
{

	// get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((allowed_types & (1 << i))	
			// index of memory type must match corresponding bit in allowedTypes
			// desired property bit flags are part of memory type's property flags
			&& (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {																			

			// this memory type is valid, so return its index
			return i;

		}

	}

	return -1;

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

static VkAccessFlags access_flags_for_image_layout(VkImageLayout layout) {

	switch (layout)
	{
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_SHADER_READ_BIT;
	default:
		return VkAccessFlags();
	}

}

static VkPipelineStageFlags pipeline_stage_for_layout(VkImageLayout oldImageLayout) {

	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow queue other than graphic
													// return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow queue other than graphic
													// return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	default:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

}

// aligned piece of memory appropiately and when necessary return bigger piece
static uint32_t align_up(uint32_t memory, uint32_t alignment) {
	return (memory + alignment - 1) & ~(alignment - 1);
}