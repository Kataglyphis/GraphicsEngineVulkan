#include "Texture.h"
#include <stdexcept>

Texture::Texture()
{
}

void Texture::createFromFile(VulkanDevice* device, VkCommandPool commandPool, std::string fileName)
{
	int width, height;
	VkDeviceSize size;
	stbi_uc* image_data = loadTextureData(fileName, &width, &height, &size);

	mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	// create staging buffer to hold loaded data, ready to copy to device
	VulkanBuffer stagingBuffer;
	stagingBuffer.create(device, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// copy image data to staging buffer 
	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getBufferMemory(), 0, size, 0, &data);
	memcpy(data, image_data, static_cast<size_t>(size));
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getBufferMemory());

	// free original image data
	stbi_image_free(image_data);

	createImage(device,
				width, height, mip_levels,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// copy data to image
	// transition image to be DST for copy operation
	transition_image_layout(device->getLogicalDevice(), device->getGraphicsQueue(),
					commandPool, vulkanImage.getImage(),
							VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

	// copy data to image
	copy_image_buffer(	device->getLogicalDevice(), device->getGraphicsQueue(), 
		commandPool, stagingBuffer.getBuffer(),
						vulkanImage.getImage(), width, height);

	// generate mipmaps
	generate_mipmaps(	device->getPhysicalDevice(), device->getLogicalDevice(),
				commandPool, device->getGraphicsQueue(),
						vulkanImage.getImage(), VK_FORMAT_R8G8B8A8_SRGB,
						width, height, mip_levels);

	stagingBuffer.cleanUp();

	createImageView(device,
					VK_FORMAT_R8G8B8A8_UNORM,
					VK_IMAGE_ASPECT_COLOR_BIT, 
					mip_levels);

}

void Texture::setImage(VkImage image)
{
	vulkanImage.setImage(image);
}

void Texture::setImageView(VkImageView imageView)
{
	vulkanImageView.setImageView(imageView);
}

void Texture::createImage(	VulkanDevice* device,
							uint32_t width, uint32_t height,
							uint32_t mip_levels, VkFormat format,
							VkImageTiling tiling,
							VkImageUsageFlags use_flags,
							VkMemoryPropertyFlags prop_flags)
{
	vulkanImage.create(	device,
						width, height,
						mip_levels, format,
						tiling,
						use_flags,
						prop_flags);
}

void Texture::createImageView(	VulkanDevice* device, 
								VkFormat format, 
								VkImageAspectFlags aspect_flags, 
								uint32_t mip_levels)
{
	vulkanImageView.create(	device,
							vulkanImage.getImage(), 
							format,
							aspect_flags,
							mip_levels);
}

void Texture::cleanUp()
{
	vulkanImageView.cleanUp();
	vulkanImage.cleanUp();
}

Texture::~Texture()
{
}

stbi_uc* Texture::loadTextureData(	std::string file_name, 
									int* width, int* height, 
									VkDeviceSize* image_size)
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
