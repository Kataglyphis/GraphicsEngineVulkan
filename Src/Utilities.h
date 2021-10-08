#pragma once

#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

const int MAX_FRAME_DRAWS = 2;

const std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// vertex data representation (layout)
struct Vertex {

	glm::vec3 pos; // vertex position (x,y,z)

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

static uint32_t find_memory_type_index(VkPhysicalDevice physical_device, uint32_t allowed_types, VkMemoryPropertyFlags properties)
{

	// get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((allowed_types & (1 << i))																																// index of memory type must match corresponding bit in allowedTypes
			&& (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {					// desired property bit flags are part of memory type's property flags																			

			// this memory type is valid, so return its index
			return i;

		}

	}

	return -1;

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

		throw std::runtime_error("Failed to allocate memory for vertex buffer!");

	}

	// allocate memory to given vertex buffer
	vkBindBufferMemory(device, *buffer, *buffer_memory, 0);

}

static void copy_buffer(VkDevice device, VkQueue transfer_queue, VkCommandPool transfer_command_pool,
											VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size) {

	// command buffer to hold transfer commands
	VkCommandBuffer transfer_command_buffer;

	// command buffer details
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = transfer_command_pool;
	alloc_info.commandBufferCount = 1;

	// allocate command buffer from pool
	vkAllocateCommandBuffers(device, &alloc_info, & transfer_command_buffer);

	// infromation to begin the command buffer record
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;											// we are only using the command buffer once, so set up for one time submit
	
	// begin recording transfer commands
	vkBeginCommandBuffer(transfer_command_buffer, &begin_info);

	// region of data to copy from and to
	VkBufferCopy buffer_copy_region{};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = buffer_size;
	
	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transfer_command_buffer, src_buffer, dst_buffer, 1, &buffer_copy_region);

	// end commands
	vkEndCommandBuffer(transfer_command_buffer);

	// queue submission information 
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &transfer_command_buffer;

	// submit transfer command to transfer queue and wait until it finishes
	vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(transfer_queue);

	// free temporary command buffer back to pool
	vkFreeCommandBuffers(device, transfer_command_pool, 1, &transfer_command_buffer);
}