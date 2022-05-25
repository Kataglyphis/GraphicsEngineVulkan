#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <stb_image.h>

#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"

#include "Utilities.h"
#include <VulkanBufferManager.h>

class Texture
{
public:

	Texture();

	void				createFromFile(	VulkanDevice* device, 
										VkCommandPool commandPool, 
										std::string fileName);

	void				setImage(VkImage image);
	void				setImageView(VkImageView imageView);

	VulkanImage&		getVulkanImage() { return vulkanImage; };
	VulkanImageView&	getVulkanImageView() { return vulkanImageView; };
	VkImage&			getImage() { return vulkanImage.getImage(); };
	VkImageView&		getImageView() { return vulkanImageView.getImageView(); };

	void				createImage(	VulkanDevice* device,
										uint32_t width, uint32_t height,
										uint32_t mip_levels, VkFormat format,
										VkImageTiling tiling,
										VkImageUsageFlags use_flags,
										VkMemoryPropertyFlags prop_flags);

	void				createImageView(	VulkanDevice* device,
											VkFormat format, VkImageAspectFlags aspect_flags,
											uint32_t mip_levels);

	void				cleanUp();

	~Texture();

private:

	int					mip_levels = 0;

	stbi_uc*			loadTextureData(	std::string file_name, 
											int* width, int* height, 
											VkDeviceSize* image_size);

	void				generateMipMaps(	VkPhysicalDevice physical_device, VkDevice device,
											VkCommandPool command_pool,
											VkQueue queue, VkImage image, VkFormat image_format,
											int32_t width, int32_t height, uint32_t mip_levels);

	VulkanBufferManager vulkanBufferManager;

	VulkanImage			vulkanImage;
	VulkanImageView		vulkanImageView;

};

