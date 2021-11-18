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

	Mesh(VkDevice logical_device, VkPhysicalDevice physical_device, VkDevice device, VkQueue transfer_queue,
				VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
				int new_texture_id);

	void set_model(glm::mat4 new_model);
	Model get_model();
	ObjectDescription get_object_description();

	int get_texture_id();

	int get_vertex_count();
	int get_index_count();
	VkBuffer get_vertex_buffer();
	VkBuffer get_index_buffer();

	void destroy_buffers();

	~Mesh();

private:

	ObjectDescription object_description;

	Model model;

	int texture_id;

	int vertex_count;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;

	int index_count;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkBuffer object_description_buffer;
	VkDeviceMemory object_description_buffer_memory;

	VkPhysicalDevice physical_device;
	VkDevice device;

	void create_vertex_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices);
	void create_index_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<uint32_t>* indices);

};