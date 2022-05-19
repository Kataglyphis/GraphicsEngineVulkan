#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "Utilities.h"
#include "QueueFamilyIndices.h"

class VulkanDevice
{

public:

	VulkanDevice(VkInstance* instance, VkSurfaceKHR* surface);

	VkPhysicalDevice	getPhysicalDevice() const { return physical_device; };
	VkDevice			getLogicalDevice() const { return logical_device; };
	QueueFamilyIndices	getQueueFamilies();
	VkQueue				getGraphicsQueue() const { return graphics_queue; };
	VkQueue				getComputeQueue() const { return compute_queue; };
	VkQueue				getPresentationQueue() const { return presentation_queue; };
	SwapChainDetails	getSwapchainDetails();

	~VulkanDevice();

private:

	VkPhysicalDevice			physical_device;
	VkPhysicalDeviceProperties	device_properties;

	VkDevice					logical_device;

	VkInstance*					instance;
	VkSurfaceKHR*				surface;

	// available queues
	VkQueue						graphics_queue;
	VkQueue						presentation_queue;
	VkQueue						compute_queue;

	void				get_physical_device();
	void				create_logical_device();

	QueueFamilyIndices	getQueueFamilies(VkPhysicalDevice physical_device);
	SwapChainDetails	getSwapchainDetails(VkPhysicalDevice device);

	bool				check_device_suitable(VkPhysicalDevice device);
	bool				check_device_extension_support(VkPhysicalDevice device);

	const std::vector<const char*> device_extensions = {

								VK_KHR_SWAPCHAIN_EXTENSION_NAME

	};
};

