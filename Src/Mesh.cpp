#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physical_device, VkDevice device, std::vector<Vertex>* vertices)
{
	vertex_count = vertices->size();
	this->physical_device = physical_device;
	this->device = device;
	vertex_buffer = create_vertex_buffer(vertices);
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

}

Mesh::~Mesh()
{
}

VkBuffer Mesh::create_vertex_buffer(std::vector<Vertex>* vertices)
{
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

}
