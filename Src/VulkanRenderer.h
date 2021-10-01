#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <set>

#include "MyWindow.h"
#include "Utilities.h"


class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(std::shared_ptr<MyWindow> window);
	void clean_up();

	~VulkanRenderer();

private:

	// use the standard validation layers from the SDK for error checking
	const std::vector<const char*> validationLayers = {
						"VK_LAYER_KHRONOS_validation"
	};

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	//wrapper class for GLFWwindow
	std::shared_ptr<MyWindow> window;

	//Vulkan components
	VkInstance instance;

	struct {
		VkPhysicalDevice physical_device;
		VkDevice logical_device;
	} MainDevice;

	VkQueue graphics_queue;
	VkQueue presentation_queue;

	// surface defined on windows as WIN32 window system, Linux f.e. X11, MacOS also their own
	VkSurfaceKHR surface; 

	//Vulkan functions
	// all create functions
	void create_instance();
	void create_logical_device();
	void create_surface();

	//get functions
	void get_physical_device();

	// support functions
	// checker functions
	bool check_instance_extension_support(std::vector<const char*>* check_extensions);
	bool check_device_extension_support(VkPhysicalDevice device);
	bool check_device_suitable(VkPhysicalDevice device);

	// getter functions
	QueueFamilyIndices get_queue_families(VkPhysicalDevice device);
	SwapChainDetails get_swapchain_details(VkPhysicalDevice device);

	//validation layers
	bool checkValidationLayerSupport();

};

