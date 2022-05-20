#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Utilities.h"
#include "VulkanDebug.h"

class VulkanInstance
{

public:

	VulkanInstance();

	VkInstance&			getVulkanInstance() { return instance; };

	~VulkanInstance();

private:

	VkInstance		instance;
	VulkanDebug		vulkanDebug;

	bool			check_validation_layer_support();
	bool			check_instance_extension_support(std::vector<const char*>* check_extensions);
};
