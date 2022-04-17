#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "Utilities.h"

class ImageHelper
{

public:

	ImageHelper(VkPhysicalDevice physical_device, VkDevice logical_device);

	VkImage create_image(	uint32_t width, uint32_t height, uint32_t mip_levels, 
							VkFormat format, VkImageTiling tiling, 
							VkImageUsageFlags use_flags,
							VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory);

	VkImageView create_image_view(	VkImage image, VkFormat format, 
									VkImageAspectFlags aspect_flags, uint32_t mip_levels);

private:

	VkPhysicalDevice physical_device; 
	VkDevice logical_device;

};

