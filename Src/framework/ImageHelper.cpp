#include "ImageHelper.h"

ImageHelper::ImageHelper(VkPhysicalDevice physical_device, VkDevice logical_device)
{
	this->physical_device = physical_device;
	this->logical_device = logical_device;
}

VkImage ImageHelper::create_image(	uint32_t width, uint32_t height, uint32_t mip_levels,
									VkFormat format, VkImageTiling tiling, VkImageUsageFlags use_flags, 
									VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory)
{
	// CREATE image
	// image creation info
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;													// type of image (1D, 2D, 3D)
	image_create_info.extent.width = width;																		// width if image extent
	image_create_info.extent.height = height;																	// height if image extent
	image_create_info.extent.depth = 1;																				// height if image extent
	image_create_info.mipLevels = mip_levels;																	// number of mipmap levels 
	image_create_info.arrayLayers = 1;																				// number of levels in image array
	image_create_info.format = format;																				// format type of image 
	image_create_info.tiling = tiling;																					// tiling of image ("arranged" for optimal reading)
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;							// layout of image data on creation 
	image_create_info.usage = use_flags;																			// bit flags defining what image will be used for
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;											// number of samples for multisampling 
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;							// whether image can be shared between queues

	VkImage image;
	VkResult result = vkCreateImage(logical_device, &image_create_info, nullptr, &image);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create an image!");

	}

	// CREATE memory for image
	// get memory requirements for a type of image
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);

	// allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;
	memory_alloc_info.memoryTypeIndex = find_memory_type_index(physical_device, memory_requirements.memoryTypeBits,
		prop_flags);

	result = vkAllocateMemory(logical_device, &memory_alloc_info, nullptr, image_memory);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate memory!");

	}

	// connect memory to image
	vkBindImageMemory(logical_device, image, *image_memory, 0);

	return image;
}

VkImageView ImageHelper::create_image_view(	VkImage image, VkFormat format, 
											VkImageAspectFlags aspect_flags, uint32_t mip_levels)
{
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
	VkResult result = vkCreateImageView(logical_device, &view_create_info, nullptr, &image_view);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create an image view!");

	}

	return image_view;
}
