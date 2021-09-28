#pragma once

# define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include "MyWindow.h"
#include <memory>

class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(std::shared_ptr<MyWindow> window);

	~VulkanRenderer();

private:

	std::shared_ptr<MyWindow> window;

	//Vulkan components
	VkInstance instance;

	//Vulkan functions
	void create_instance();

};

