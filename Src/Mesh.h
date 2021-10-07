# pragma once 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

class Mesh {

public:

	Mesh();
	Mesh(VkPhysicalDevice physical_device, VkDevice device, std::vector<Vertex>* vertices);

	int get_vertex_count();
	VkBuffer get_vertex_buffer();

	void destroy_vertex_buffer();

	~Mesh();

private:

	int vertex_count;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;

	VkPhysicalDevice physical_device;
	VkDevice device;

	void create_vertex_buffer(std::vector<Vertex>* vertices);
	uint32_t find_memory_type_index(uint32_t allowed_types, VkMemoryPropertyFlags properties);

};