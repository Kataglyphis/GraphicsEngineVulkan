#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physical_device, VkDevice device, std::vector<Vertex>* vertices)
{
	vertex_count = static_cast<uint32_t>(vertices->size());
	this->physical_device = physical_device;
	this->device = device;
	create_vertex_buffer(vertices);
}

int Mesh::get_vertex_count()
{
	return vertex_count;
}

VkBuffer Mesh::get_vertex_buffer()
{
	return vertex_buffer;
}

void Mesh::destroy_vertex_buffer()
{

	vkDestroyBuffer(device, vertex_buffer, nullptr);
	vkFreeMemory(device, vertex_buffer_memory, nullptr);

}

Mesh::~Mesh()
{
}

void Mesh::create_vertex_buffer(std::vector<Vertex>* vertices)
{

	// create vertex buffer


	// information to create a buffer (doesn't include assigning memory)
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = sizeof(Vertex) * vertices->size();										// size of buffer (size of 1 vertex * #vertices)
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;				// multiple types of buffer possible, we want Vertex buffer		
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;						// similar to swap chain images, can share vertex buffers
	
	VkResult result = vkCreateBuffer(device, &buffer_info, nullptr, &vertex_buffer);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create vertex buffer!");

	}

	// get buffer memory requirements
	VkMemoryRequirements memory_requirements{};
	vkGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);

	// allocate memory to buffer
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;

	uint32_t memory_type_index = find_memory_type_index(memory_requirements.memoryTypeBits,
																										VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |		/* memory is visible to CPU side */
																										VK_MEMORY_PROPERTY_HOST_COHERENT_BIT	/* data is placed straight into buffer */);
	if (memory_type_index < 0) {

		throw std::runtime_error("Failed to find auitable memory type!");

	}

	memory_alloc_info.memoryTypeIndex = memory_type_index;

	// allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device, &memory_alloc_info, nullptr, &vertex_buffer_memory);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate memory for vertex buffer!");

	}

	// allocate memory to given vertex buffer
	vkBindBufferMemory(device, vertex_buffer, vertex_buffer_memory, 0);

	// Map memory to vertex buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(device, vertex_buffer_memory, 0, buffer_info.size, 0, &data);						// 2.) map the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t) buffer_info.size);														// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(device, vertex_buffer_memory);																	// 4.) unmap the vertex buffer memory

}

uint32_t Mesh::find_memory_type_index(uint32_t allowed_types, VkMemoryPropertyFlags properties)
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
