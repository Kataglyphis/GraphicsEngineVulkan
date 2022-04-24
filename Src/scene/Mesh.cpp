#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physical_device, VkDevice logical_device, VkQueue transfer_queue,
	VkCommandPool transfer_command_pool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
	std::vector<unsigned int>& materialIndex)
{

	VkTransformMatrixKHR transform_matrix{};
	// glm uses column major matrices so transpose it for Vulkan want row major here
	glm::mat4 transpose_transform = glm::transpose(glm::mat4(1.0f));
	VkTransformMatrixKHR out_matrix;
	memcpy(&out_matrix, &transpose_transform, sizeof(VkTransformMatrixKHR));

	index_count = static_cast<uint32_t>(indices.size());
	vertex_count = static_cast<uint32_t>(vertices.size());
	this->physical_device = physical_device;
	this->device = logical_device;
	object_description = ObjectDescription{};
	create_vertex_buffer(transfer_queue, transfer_command_pool, &vertices);
	create_index_buffer(transfer_queue, transfer_command_pool, &indices);
	create_material_id_buffer(transfer_queue, transfer_command_pool, &materialIndex);

	VkBufferDeviceAddressInfo vertex_info{};
	vertex_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	vertex_info.buffer = vertex_buffer;

	VkBufferDeviceAddressInfo index_info{};
	index_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	index_info.buffer = index_buffer;

	VkBufferDeviceAddressInfo material_index_info{};
	material_index_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	material_index_info.buffer = material_ids_buffer;

	object_description.index_address = vkGetBufferDeviceAddress(logical_device, &index_info);
	object_description.vertex_address = vkGetBufferDeviceAddress(logical_device, &vertex_info);
	object_description.material_index_address = vkGetBufferDeviceAddress(logical_device, &material_index_info);

	model = glm::mat4(1.0f);

}

void Mesh::set_model(glm::mat4 new_model)
{
	model = new_model;
}

glm::mat4 Mesh::get_model()
{
	return model;
}

ObjectDescription Mesh::get_object_description()
{
	return object_description;
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

VkBuffer Mesh::get_materialID_buffer()
{
	return material_ids_buffer;
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
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_buffer, &staging_buffer_memory);

	// Map memory to vertex buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)buffer_size);																// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
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
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
		&index_buffer,
		&index_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device, transfer_queue, transfer_command_pool, staging_buffer, index_buffer, buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);

}

void Mesh::create_material_id_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<unsigned int>* materialIndex)
{

	// get size of buffer need
	VkDeviceSize buffer_size = sizeof(uint32_t) * materialIndex->size();

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
	memcpy(data, materialIndex->data(), (size_t)buffer_size);																	// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(physical_device, device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
		&material_ids_buffer,
		&material_ids_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device, transfer_queue, transfer_command_pool, staging_buffer, material_ids_buffer, buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);

}
