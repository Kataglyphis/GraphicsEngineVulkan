#include "Texture.h"

Texture::Texture(VkPhysicalDevice physical_device, VkDevice logical_device, VkQueue queue, VkCommandPool command_pool)
{
	this->physical_device = physical_device;
	this->logical_device = logical_device;
	this->queue = queue;
	this->command_pool = command_pool;
}

void Texture::create(std::string filename)
{
	ImageHelper image_helper{ physical_device, logical_device };
	// create texture image and get its location in array
	create_texture_image(filename);

	// create image view and add to list
	texture_image_view = image_helper.create_image_view(texture_image, VK_FORMAT_R8G8B8A8_UNORM,
															VK_IMAGE_ASPECT_COLOR_BIT,
															1);

}

void Texture::create_texture_image(std::string filename)
{
	ImageHelper image_helper{ physical_device, logical_device };

	int width, height;
	VkDeviceSize image_size;
	stbi_uc* image_data = load_texture_file(filename, &width, &height, &image_size);

	// find the number of mip level we want to create 
	int mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	// create staging buffer to hold loaded data, ready to copy to device
	VkBuffer image_staging_buffer;
	VkDeviceMemory image_staging_buffer_memory;
	create_buffer(physical_device, logical_device, image_size,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					&image_staging_buffer, &image_staging_buffer_memory);

	// copy image data to staging buffer 
	void* data;
	vkMapMemory(logical_device, image_staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, image_data, static_cast<size_t>(image_size));
	vkUnmapMemory(logical_device, image_staging_buffer_memory);

	// free original image data
	stbi_image_free(image_data);

	texture_image = image_helper.create_image(width, height, mip_levels, VK_FORMAT_R8G8B8A8_UNORM,
												VK_IMAGE_TILING_OPTIMAL,
												VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
												VK_IMAGE_USAGE_TRANSFER_DST_BIT |
												VK_IMAGE_USAGE_SAMPLED_BIT,
												VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
												&texture_image_memory);

	// copy data to image
	// transition image to be DST for copy operation
	transition_image_layout(logical_device, queue, command_pool, texture_image, VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

	// copy data to image
	copy_image_buffer(	logical_device, queue, command_pool, image_staging_buffer,
						texture_image, width, height);

	// transition image to be shader readable for shader stage

	//transition_image_layout(MainDevice.logical_device, graphics_queue, graphics_command_pool, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//											VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);

	// destroy staging buffers
	vkDestroyBuffer(logical_device, image_staging_buffer, nullptr);
	vkFreeMemory(logical_device, image_staging_buffer_memory, nullptr);

	// generate mipmaps
	generate_mipmaps(physical_device, logical_device, command_pool, queue,
					texture_image, VK_FORMAT_R8G8B8A8_SRGB, width, height, mip_levels);

}

stbi_uc* Texture::load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size)
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
