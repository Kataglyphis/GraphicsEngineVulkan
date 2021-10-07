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

	VkPhysicalDevice physical_device;
	VkDevice device;

	VkBuffer create_vertex_buffer(std::vector<Vertex>* vertices);

};