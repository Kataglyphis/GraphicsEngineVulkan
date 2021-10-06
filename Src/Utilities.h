#pragma once

#include <fstream>

const std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices {

	int graphics_family = -1;																// location of graphics family
	int presentation_family = -1;														// location of presentation queue family

	//check if queue families are valid 
	bool is_valid() {

		return graphics_family >= 0 && presentation_family >= 0;

	}

};

struct SwapChainDetails {

	VkSurfaceCapabilitiesKHR surface_capabilities;				// surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;					// surface image formats, e.g. RGBA and size of each color
	std::vector<VkPresentModeKHR> presentation_mode;  // how images should be presented to screen

};

struct SwapChainImage {

	VkImage image;
	VkImageView image_view;

};

static std::vector<char> read_file(const std::string& filename) {

	// open stream from given file 
	// std::ios::binary tells stream to read file as binary
	// std::ios:ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// check if file stream sucessfully opened
	if (!file.is_open()) {

		throw std::runtime_error("Failed to open a file!");

	}

	size_t file_size = (size_t) file.tellg();
	std::vector<char> file_buffer(file_size);

	// move read position to start of file 
	file.seekg(0);

	// read the file data into the buffer (stream "file_size" in total)
	file.read(file_buffer.data(), file_size);

	file.close();

	return file_buffer;
}
