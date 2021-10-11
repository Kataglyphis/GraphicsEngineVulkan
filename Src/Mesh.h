# pragma once 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>

#include "Utilities.h"

struct Model {
	glm::mat4 model;
};

class Mesh {

public:

	Mesh();
	Mesh(VkPhysicalDevice physical_device, VkDevice device, VkQueue transfer_queue, 
				VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices);

	void set_model(glm::mat4 new_model);
	Model get_model();

	int get_vertex_count();
	int get_index_count();
	VkBuffer get_vertex_buffer();
	VkBuffer get_index_buffer();

	void destroy_buffers();

	~Mesh();

private:

	Model model;

	int vertex_count;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;

	int index_count;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkPhysicalDevice physical_device;
	VkDevice device;

	void create_vertex_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices);
	void create_index_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<uint32_t>* indices);

};