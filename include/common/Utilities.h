#pragma once

#include <fstream>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "host_device_shared_vars.h"
#include "GlobalValues.h"

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

static void copy_image_buffer(	VkDevice device, 
								VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
								VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height) {

	// create buffer
	VkCommandBuffer transfer_command_buffer = begin_command_buffer(device, transfer_command_pool);

	VkBufferImageCopy image_region{};
	image_region.bufferOffset = 0;												// offset into data
	image_region.bufferRowLength = 0;											// row length of data to calculate data spacing
	image_region.bufferImageHeight = 0;											// image height to calculate data spacing
	image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// which aspect of image to copy
	image_region.imageSubresource.mipLevel = 0;
	image_region.imageSubresource.baseArrayLayer = 0;
	image_region.imageSubresource.layerCount = 1;
	image_region.imageOffset = {0, 0, 0};										// offset into image 
	image_region.imageExtent = {width, height, 1};

	// copy buffer to given image
	vkCmdCopyBufferToImage(transfer_command_buffer, src_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_region);

	end_and_submit_command_buffer(device, transfer_command_pool, transfer_queue, transfer_command_buffer);

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

static void transition_image_layout(VkDevice device, VkQueue queue, 
									VkCommandPool command_pool, VkImage image, VkImageLayout old_layout,
									VkImageLayout new_layout, uint32_t mip_levels) {

	VkCommandBuffer command_buffer = begin_command_buffer(device, command_pool);

	VkImageMemoryBarrier memory_barrier{};
	memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memory_barrier.oldLayout = old_layout;
	memory_barrier.newLayout = new_layout;
	memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family to transition from 
	memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family to transition to
	memory_barrier.image = image;												// image being accessed and modified as part of barrier
	memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// aspect of image being altered
	memory_barrier.subresourceRange.baseMipLevel = 0;							// first mip level to start alterations on
	memory_barrier.subresourceRange.levelCount = mip_levels;					// number of mip levels to alter starting from baseMipLevel
	memory_barrier.subresourceRange.baseArrayLayer = 0;							// first layer to start alterations on
	memory_barrier.subresourceRange.layerCount = 1;								// number of layers to alter starting from baseArrayLayer

	// if transitioning from new image to image ready to receive data
	memory_barrier.srcAccessMask = access_flags_for_image_layout(old_layout);
	memory_barrier.dstAccessMask = access_flags_for_image_layout(new_layout);

	VkPipelineStageFlags src_stage = pipeline_stage_for_layout(old_layout);
	VkPipelineStageFlags dst_stage = pipeline_stage_for_layout(new_layout);

	vkCmdPipelineBarrier(

		command_buffer,
		src_stage, dst_stage,				// pipeline stages (match to src and dst accessmask)
		0,									// no dependency flags
		0, nullptr,							// memory barrier count + data
		0, nullptr,							// buffer memory barrier count + data
		1, &memory_barrier					// image memory barrier count + data

	);

	end_and_submit_command_buffer(device, command_pool, queue, command_buffer);

}

static void transition_image_layout_for_command_buffer(	VkCommandBuffer command_buffer, VkImage image, 
														VkImageLayout old_layout,
														VkImageLayout new_layout, uint32_t mip_levels, 
														VkImageAspectFlags aspectMask) {

	VkImageMemoryBarrier memory_barrier{};
	memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memory_barrier.oldLayout = old_layout;
	memory_barrier.newLayout = new_layout;
	memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family to transition from 
	memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family to transition to
	memory_barrier.image = image;										// image being accessed and modified as part of barrier
	memory_barrier.subresourceRange.aspectMask = aspectMask;			// aspect of image being altered
	memory_barrier.subresourceRange.baseMipLevel = 0;					// first mip level to start alterations on
	memory_barrier.subresourceRange.levelCount = mip_levels;			// number of mip levels to alter starting from baseMipLevel
	memory_barrier.subresourceRange.baseArrayLayer = 0;					// first layer to start alterations on
	memory_barrier.subresourceRange.layerCount = 1;						// number of layers to alter starting from baseArrayLayer

	memory_barrier.srcAccessMask = access_flags_for_image_layout(old_layout);
	memory_barrier.dstAccessMask = access_flags_for_image_layout(new_layout);

	VkPipelineStageFlags src_stage = pipeline_stage_for_layout(old_layout);
	VkPipelineStageFlags dst_stage = pipeline_stage_for_layout(new_layout);

	// if transitioning from new image to image ready to receive data
	

	vkCmdPipelineBarrier(

		command_buffer,
		src_stage, dst_stage,				// pipeline stages (match to src and dst accessmask)
		0,									// no dependency flags
		0, nullptr,							// memory barrier count + data
		0, nullptr,							// buffer memory barrier count + data
		1, &memory_barrier					// image memory barrier count + data

	);

}

// we have to create mipmap levels in staging buffers by our own
static void generate_mipmaps(	VkPhysicalDevice physical_device, VkDevice device, 
								VkCommandPool command_pool,
								VkQueue queue, VkImage image, VkFormat image_format,
								int32_t width, int32_t height, uint32_t mip_levels) {

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physical_device, image_format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer command_buffer = begin_command_buffer(device, command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	
	// TEMP VARS needed for decreasing step by step for factor 2
	int32_t tmp_width = width;
	int32_t tmp_height = height;

	// -- WE START AT 1 ! 
	for (uint32_t i = 1; i < mip_levels; i++) {

		// WAIT for previous mip map level for being ready
		barrier.subresourceRange.baseMipLevel = i - 1;
		// HERE we TRANSITION for having a SRC format now 
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		
		// when barrier over we can now blit :)
		VkImageBlit blit{};

		// -- OFFSETS describing the 3D-dimesnion of the region 
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { tmp_width, tmp_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// copy from previous level
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		// -- OFFSETS describing the 3D-dimesnion of the region 
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { tmp_width > 1 ? tmp_width / 2 : 1, tmp_height > 1 ? tmp_height / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// -- COPY to next mipmap level
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command_buffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		// REARRANGE image formats for having the correct image formats again
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (tmp_width > 1) tmp_width /= 2;
		if (tmp_height > 1) tmp_height /= 2;

	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	end_and_submit_command_buffer(device, command_pool, queue, command_buffer);

}

// aligned piece of memory appropiately and when necessary return bigger piece
static uint32_t align_up(uint32_t memory, uint32_t alignment) {
	return (memory + alignment - 1) & ~(alignment - 1);
}