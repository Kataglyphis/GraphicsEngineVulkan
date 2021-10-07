#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <set>
#include <algorithm>
#include <array>

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

	VkSwapchainKHR swapchain;
	std::vector<SwapChainImage> swap_chain_images;
	std::vector<VkFramebuffer> swap_chain_framebuffers;
	std::vector<VkCommandBuffer> command_buffers;

	// --PIPELINE --
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;

	// -- POOLS --
	VkCommandPool graphics_command_pool;

	// utilities
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;

	//Vulkan functions
	// all create functions
	void create_instance();
	void create_logical_device();
	void create_surface();
	void create_swap_chain();
	void create_render_pass();
	void create_graphics_pipeline();
	void create_framebuffers();
	void create_command_pool();
	void create_command_buffers();

	// - record functions
	void record_commands();

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

	// choose functions
	VkSurfaceFormatKHR choose_best_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR choose_best_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities);

	// create functions
	VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
	VkShaderModule create_shader_module(const std::vector<char>& code);

};

