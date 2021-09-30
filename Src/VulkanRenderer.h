#pragma once

# define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>

#include "MyWindow.h"
#include "Utilities.h"

class VulkanRenderer
{
public:

	VulkanRenderer();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																												VkDebugUtilsMessageTypeFlagsEXT messageType,
																												const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
																												void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	int init(std::shared_ptr<MyWindow> window);
	void clean_up();

	~VulkanRenderer();

private:

	//wrapper class for GLFWwindow
	std::shared_ptr<MyWindow> window;

	// use the standard validation layers from the SDK for error checking
	const std::vector<const char*> validationLayers = {
						"VK_LAYER_KHRONOS_validation"
	};

	//Vulkan components
	VkInstance instance;
	//specific messenger for debugging; we are responsible for creating and destroying
	VkDebugUtilsMessengerEXT debugMessenger;

	struct {
		VkPhysicalDevice physical_device;
		VkDevice logical_device;
	} MainDevice;

	VkQueue graphics_queue;

	//Vulkan functions
	// all create functions
	void create_instance();
	void create_logical_device();
	void create_debug_messenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	//get functions
	void get_physical_device();

	// support functions
	// checker functions
	bool check_instance_extension_support(std::vector<const char*>* check_extensions);
	bool check_device_suitable(VkPhysicalDevice device);

	// getter functions
	QueueFamilyIndices get_queue_families(VkPhysicalDevice device);

	//validation layers
	bool checkValidationLayerSupport();
};

