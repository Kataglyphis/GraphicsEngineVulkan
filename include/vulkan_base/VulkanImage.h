#pragma once
#include <vulkan/vulkan.h>

#include "VulkanDevice.h"

class VulkanImage
{
public:
	
	VulkanImage();

	void create(VulkanDevice* device,
				uint32_t width, uint32_t height,
				uint32_t mip_levels, VkFormat format,
				VkImageTiling tiling,
				VkImageUsageFlags use_flags,
				VkMemoryPropertyFlags prop_flags);

	void		setImage(VkImage image);
	VkImage&	getImage() { return image; };

	void		cleanUp();

	~VulkanImage();

private:

	VulkanDevice*	device;

	VkImage			image;
	VkDeviceMemory	imageMemory;

};

