#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

class VulkanImageView
{
public:

	VulkanImageView();

	VkImageView getImageView() { return imageView; };

	void		create(	VulkanDevice* device, VkImage image,
						VkFormat format, VkImageAspectFlags aspect_flags,
						uint32_t mip_levels);

	void		cleanUp();

	~VulkanImageView();

private:

	VulkanDevice*	device;

	VkImageView		imageView;

};

