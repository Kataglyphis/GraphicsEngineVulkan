#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physical_device, VkDevice device, VkQueue transfer_queue,
						VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
						int new_texture_id)
{

	index_count = static_cast<uint32_t>(indices->size());
	vertex_count = static_cast<uint32_t>(vertices->size());
	this->physical_device = physical_device;
	this->device = device;
	create_vertex_buffer(transfer_queue, transfer_command_pool, vertices);
	create_index_buffer(transfer_queue, transfer_command_pool, indices);

	model.model = glm::mat4(1.0f);
	texture_id = new_texture_id;

}

void Mesh::set_model(glm::mat4 new_model)
{

	model.model = new_model;

}

Model Mesh::get_model()
{
	return model;
}

int Mesh::get_texture_id()
{
	return texture_id;
}

int Mesh::get_vertex_count()
{
	return vertex_count;
}

int Mesh::get_index_count()
{
	return index_count;
}

VkBuffer Mesh::get_vertex_buffer()
{
	return vertex_buffer;
}

VkBuffer Mesh::get_index_buffer()
{
	return index_buffer;
}

void Mesh::destroy_buffers()
{

	vkDestroyBuffer(device, vertex_buffer, nullptr);
	vkFreeMemory(device, vertex_buffer_memory, nullptr);

	vkDestroyBuffer(device, index_buffer, nullptr);
	vkFreeMemory(device, index_buffer_memory, nullptr);

}

Mesh::~Mesh()
{
}

void Mesh::create_vertex_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices)
{

	VkDeviceSize buffer_size = sizeof(Vertex) * vertices->size();

	// temporary buffer to "stage" vertex data before transfering to GPU
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	// create buffer and allocate memory to it
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
												&staging_buffer, &staging_buffer_memory);

	// Map memory to vertex buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t) buffer_size);																// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
																								VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
																								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																								&vertex_buffer,
																								&vertex_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device, transfer_queue, transfer_command_pool, staging_buffer, vertex_buffer, buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);

}

void Mesh::create_index_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<uint32_t>* indices)
{
	// get size of buffer need
	VkDeviceSize buffer_size = sizeof(uint32_t) * indices->size();

	// temporary buffer to "stage" index data before transfering to GPU
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	// create buffer and allocate memory to it
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																								&staging_buffer, &staging_buffer_memory);

	// Map memory to index buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
	memcpy(data, indices->data(), (size_t)buffer_size);																	// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
																								VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
																								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																								&index_buffer,
																								&index_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device, transfer_queue, transfer_command_pool, staging_buffer, index_buffer, buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}
