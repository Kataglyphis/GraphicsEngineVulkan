#include "Mesh.h"
#include <memory>
#include <cstring>

#include "VulkanBuffer.h"

Mesh::Mesh()
{
}

Mesh::Mesh(	VulkanDevice* device, VkQueue transfer_queue,
			VkCommandPool transfer_command_pool, 
			std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
			std::vector<unsigned int>& materialIndex, std::vector<ObjMaterial>&	materials)
{

	VkTransformMatrixKHR transform_matrix{};
	// glm uses column major matrices so transpose it for Vulkan want row major here
	glm::mat4 transpose_transform = glm::transpose(glm::mat4(1.0f));
	VkTransformMatrixKHR out_matrix;
	std::memcpy(&out_matrix, &transpose_transform, sizeof(VkTransformMatrixKHR));

	index_count = static_cast<uint32_t>(indices.size());
	vertex_count = static_cast<uint32_t>(vertices.size());
	this->device = device;
	object_description = ObjectDescription{};
	create_vertex_buffer(transfer_queue, transfer_command_pool, &vertices);
	create_index_buffer(transfer_queue, transfer_command_pool, &indices);
	create_material_id_buffer(transfer_queue, transfer_command_pool, &materialIndex);
 	create_material_buffer(transfer_queue, transfer_command_pool, &materials);

	VkBufferDeviceAddressInfo vertex_info{};
	vertex_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	vertex_info.buffer = vertexBuffer.getBuffer();

	VkBufferDeviceAddressInfo index_info{};
	index_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	index_info.buffer = indexBuffer.getBuffer();

	VkBufferDeviceAddressInfo material_index_info{};
	material_index_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	material_index_info.buffer = materialIdsBuffer.getBuffer();

	VkBufferDeviceAddressInfo material_info{};
	material_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	material_info.buffer = materialsBuffer.getBuffer();

	object_description.index_address			= vkGetBufferDeviceAddress(device->getLogicalDevice(), &index_info);
	object_description.vertex_address			= vkGetBufferDeviceAddress(device->getLogicalDevice(), &vertex_info);
	object_description.material_index_address	= vkGetBufferDeviceAddress(device->getLogicalDevice(), &material_index_info);
	object_description.material_address			= vkGetBufferDeviceAddress(device->getLogicalDevice(), &material_info);

	model = glm::mat4(1.0f);

}

void Mesh::set_model(glm::mat4 new_model)
{
	model = new_model;
}

Mesh::~Mesh()
{
}

void Mesh::create_vertex_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<Vertex>* vertices)
{

	VkDeviceSize buffer_size = sizeof(Vertex) * vertices->size();

	// temporary buffer to "stage" vertex data before transfering to GPU
	VulkanBuffer staging_buffer;

	// create buffer and allocate memory to it
	staging_buffer.create(	 device, buffer_size, 
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map memory to vertex buffer
	void* data;																				
	vkMapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory(), 0, buffer_size, 0, &data);
	memcpy(data, vertices->data(), (size_t)buffer_size);						
	vkUnmapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory());

	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	vertexBuffer.create(device, buffer_size, 
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
						VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
						VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
						VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
						VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device->getLogicalDevice(), 
				transfer_queue, transfer_command_pool, 
				staging_buffer.getBuffer(), 
				vertexBuffer.getBuffer(), buffer_size);

}

void Mesh::create_index_buffer(VkQueue transfer_queue, VkCommandPool transfer_command_pool, std::vector<uint32_t>* indices)
{
	// get size of buffer need
	VkDeviceSize buffer_size = sizeof(uint32_t) * indices->size();

	// temporary buffer to "stage" vertex data before transfering to GPU
	VulkanBuffer staging_buffer;

	// create buffer and allocate memory to it
	staging_buffer.create(device, buffer_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map memory to index buffer
	void* data;																																			
	vkMapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory(), 0, buffer_size, 0, &data);
	memcpy(data, indices->data(), (size_t)buffer_size);																	
	vkUnmapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory());

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	indexBuffer.create( device, buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
							VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
							VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
							VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
							VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device->getLogicalDevice(), 
				transfer_queue, transfer_command_pool, 
				staging_buffer.getBuffer(),
				indexBuffer.getBuffer(), buffer_size);

}

void Mesh::create_material_id_buffer(	VkQueue transfer_queue, 
										VkCommandPool transfer_command_pool, 
										std::vector<unsigned int>* materialIndex)
{

	// get size of buffer need
	VkDeviceSize buffer_size = sizeof(uint32_t) * materialIndex->size();

	VulkanBuffer staging_buffer;

	staging_buffer.create(	device, buffer_size,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map memory to index buffer
	void* data;
	vkMapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory(), 0, buffer_size, 0, &data);
	memcpy(data, materialIndex->data(), (size_t)buffer_size);
	vkUnmapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory());

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	materialIdsBuffer.create( device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
								VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
								VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
								VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device->getLogicalDevice(), 
				transfer_queue, transfer_command_pool, 
				staging_buffer.getBuffer(), 
				materialIdsBuffer.getBuffer(), buffer_size);

}

void Mesh::create_material_buffer(	VkQueue transfer_queue, 
									VkCommandPool transfer_command_pool, 
									std::vector<ObjMaterial>* materials)
{

	// get size of buffer need
	VkDeviceSize buffer_size = sizeof(ObjMaterial) * materials->size();

	VulkanBuffer staging_buffer;

	staging_buffer.create(device, buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Map memory to index buffer
	void* data;
	vkMapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory(), 0, buffer_size, 0, &data);
	memcpy(data, materials->data(), (size_t)buffer_size);
	vkUnmapMemory(device->getLogicalDevice(), staging_buffer.getBufferMemory());							

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	materialsBuffer.create(device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
							VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
							VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
							VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
							VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(device->getLogicalDevice(), transfer_queue, transfer_command_pool, 
				staging_buffer.getBuffer(), materialsBuffer.getBuffer(), buffer_size);

}
