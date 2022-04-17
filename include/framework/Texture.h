#pragma once
#include <vulkan/vulkan.h>
#include "stb_image.h"
#include <string>
#include <stdexcept>
#include "Utilities.h"
#include "ImageHelper.h"
#include <algorithm>

class Texture
{

public:

	Texture(VkPhysicalDevice physical_device, VkDevice logical_device,
			VkQueue queue, VkCommandPool command_pool);

	void create(std::string filename);

private:

	void create_texture_image(std::string filename);
	stbi_uc* load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size);

	VkImage texture_image;
	VkImageView texture_image_view;
	VkDeviceMemory texture_image_memory;

	VkPhysicalDevice physical_device; 
	VkDevice logical_device;
	VkQueue queue;
	VkCommandPool command_pool;

};

