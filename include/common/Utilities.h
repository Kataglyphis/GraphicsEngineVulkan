#pragma once

#include <fstream>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "host_device_shared_vars.h"
#include "GlobalValues.h"

// Error checking on vulkan function calls
#define ASSERT_VULKAN(val,error_string)\
            if(val!=VK_SUCCESS) {\
               throw std::runtime_error(error_string); \
            }

#define NOT_YET_IMPLEMENTED throw std::runtime_error("Not yet implemented!");

static stbi_uc* load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize * image_size)
{

	// number of channels image uses
	int channels;
	// load pixel data for image
	//std::string file_loc = "../Resources/Textures/" + file_name;
	stbi_uc* image = stbi_load(file_name.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image) {

		throw std::runtime_error("Failed to load a texture file! (" + file_name + ")");

	}

	// calculate image size using given and known data
	*image_size = *width * *height * 4;

	return image;

}

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

static VkImageView create_image_view(	const VkDevice& device, VkImage image,
										VkFormat format, VkImageAspectFlags aspect_flags, 
										uint32_t mip_levels) {

	VkImageViewCreateInfo view_create_info{};
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.image = image;																	// image to create view for 
	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;												// typ of image
	view_create_info.format = format;
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;									// allows remapping of rgba components to other rgba values 
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// subresources allow the view to view only a part of an image 
	view_create_info.subresourceRange.aspectMask = aspect_flags;									// which aspect of an image to view (e.g. color bit for viewing color)
	view_create_info.subresourceRange.baseMipLevel = 0;												// start mipmap level to view from
	view_create_info.subresourceRange.levelCount = mip_levels;										// number of mipmap levels to view 
	view_create_info.subresourceRange.baseArrayLayer = 0;											// start array level to view from 
	view_create_info.subresourceRange.layerCount = 1;												// number of array levels to view 

	// create image view 
	VkImageView image_view;
	VkResult result = vkCreateImageView(device, &view_create_info, nullptr, &image_view);
	ASSERT_VULKAN(result, "Failed to create an image view!")

		return image_view;

}

static uint32_t find_memory_type_index(VkPhysicalDevice physical_device, uint32_t allowed_types, VkMemoryPropertyFlags properties)
{

	// get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((allowed_types & (1 << i))																																// index of memory type must match corresponding bit in allowedTypes
			&& (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {	// desired property bit flags are part of memory type's property flags																			

			// this memory type is valid, so return its index
			return i;

		}

	}

	return -1;

}

static VkImage create_image(VkDevice device, VkPhysicalDevice physicalDevice,
							uint32_t width, uint32_t height,
							uint32_t mip_levels, VkFormat format,
							VkImageTiling tiling,
							VkImageUsageFlags use_flags, VkMemoryPropertyFlags prop_flags,
							VkDeviceMemory* image_memory)
{

	// CREATE image
	// image creation info
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;						// type of image (1D, 2D, 3D)
	image_create_info.extent.width = width;								// width if image extent
	image_create_info.extent.height = height;							// height if image extent
	image_create_info.extent.depth = 1;									// height if image extent
	image_create_info.mipLevels = mip_levels;							// number of mipmap levels 
	image_create_info.arrayLayers = 1;									// number of levels in image array
	image_create_info.format = format;									// format type of image 
	image_create_info.tiling = tiling;																					// tiling of image ("arranged" for optimal reading)
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// layout of image data on creation 
	image_create_info.usage = use_flags;								// bit flags defining what image will be used for
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;					// number of samples for multisampling 
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// whether image can be shared between queues

	VkImage image;
	VkResult result = vkCreateImage(device, &image_create_info, nullptr, &image);
	ASSERT_VULKAN(result, "Failed to create an image!")

	// CREATE memory for image
	// get memory requirements for a type of image
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, image, &memory_requirements);

	// allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;
	memory_alloc_info.memoryTypeIndex = find_memory_type_index(physicalDevice, memory_requirements.memoryTypeBits,
		prop_flags);

	result = vkAllocateMemory(device, &memory_alloc_info, nullptr, image_memory);
	ASSERT_VULKAN(result, "Failed to allocate memory!")

	// connect memory to image
	vkBindImageMemory(device, image, *image_memory, 0);

	return image;

}


static void create_buffer(VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage_flags, 
											VkMemoryPropertyFlags buffer_propertiy_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {

	// create vertex buffer
	// information to create a buffer (doesn't include assigning memory)
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = buffer_size;																			// size of buffer (size of 1 vertex * #vertices)
	buffer_info.usage = buffer_usage_flags;															// multiple types of buffer possible, e.g. vertex buffer		
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;						// similar to swap chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &buffer_info, nullptr, buffer);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a buffer!");

	}

	// get buffer memory requirements
	VkMemoryRequirements memory_requirements{};
	vkGetBufferMemoryRequirements(device, *buffer, &memory_requirements);

	// allocate memory to buffer
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;

	uint32_t memory_type_index = find_memory_type_index(physical_device, memory_requirements.memoryTypeBits,
																											buffer_propertiy_flags);
																											//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |		/* memory is visible to CPU side */
																											//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT	/* data is placed straight into buffer */);
	if (memory_type_index < 0) {

		throw std::runtime_error("Failed to find auitable memory type!");

	}

	memory_alloc_info.memoryTypeIndex = memory_type_index;

	// allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device, &memory_alloc_info, nullptr, buffer_memory);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate memory for buffer!");

	}

	// allocate memory to given buffer
	vkBindBufferMemory(device, *buffer, *buffer_memory, 0);

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
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;											// we are only using the command buffer once, so set up for one time submit
	
	// begin recording transfer commands
	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;

}

static void end_and_submit_command_buffer(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer& command_buffer) {

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

static void copy_buffer(VkDevice device, VkQueue transfer_queue, VkCommandPool transfer_command_pool,
											VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size) {

	// create buffer
	VkCommandBuffer command_buffer = begin_command_buffer(device, transfer_command_pool);

	// region of data to copy from and to
	VkBufferCopy buffer_copy_region{};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = buffer_size;
	
	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &buffer_copy_region);

	end_and_submit_command_buffer(device, transfer_command_pool, transfer_queue, command_buffer);

}

static void copy_image_buffer(VkDevice device, VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
													VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height) {

	// create buffer
	VkCommandBuffer transfer_command_buffer = begin_command_buffer(device, transfer_command_pool);

	VkBufferImageCopy image_region{};
	image_region.bufferOffset = 0;																						// offset into data
	image_region.bufferRowLength = 0;																				// row length of data to calculate data spacing
	image_region.bufferImageHeight = 0;																			// image height to calculate data spacing
	image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// which aspect of image to copy
	image_region.imageSubresource.mipLevel = 0;
	image_region.imageSubresource.baseArrayLayer = 0;
	image_region.imageSubresource.layerCount = 1;
	image_region.imageOffset = {0, 0, 0};																			// offset into image 
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

static void transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkImage image, VkImageLayout old_layout,
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
		0,													// no dependency flags
		0, nullptr,									// memory barrier count + data
		0, nullptr,									// buffer memory barrier count + data
		1, &memory_barrier				// image memory barrier count + data

	);

	end_and_submit_command_buffer(device, command_pool, queue, command_buffer);

}

static void transition_image_layout_for_command_buffer(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout,
											VkImageLayout new_layout, uint32_t mip_levels, VkImageAspectFlags aspectMask) {

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
		0,													// no dependency flags
		0, nullptr,									// memory barrier count + data
		0, nullptr,									// buffer memory barrier count + data
		1, &memory_barrier				// image memory barrier count + data

	);

}

// we have to create mipmap levels in staging buffers by our own
static void generate_mipmaps(VkPhysicalDevice physical_device, VkDevice device, VkCommandPool command_pool,
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