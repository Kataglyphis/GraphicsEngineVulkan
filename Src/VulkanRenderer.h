#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// the importer from the assimp library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <set>
#include <algorithm>
#include <array>
#include <stdlib.h>
#include <stdio.h>

#include "stb_image.h"

#include "MyWindow.h"
#include "Utilities.h"
#include "Mesh.h"
#include "MeshModel.h"
#include "Camera.h"

// all IMGUI stuff
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

class VulkanRenderer
{
public:

	VulkanRenderer();

	int init(std::shared_ptr<MyWindow> window, glm::vec3 eye, float near_plane, float far_plane);

	int create_mesh_model(std::string model_file);
	void update_model(int model_id, glm::mat4 new_model);
	void update_view(glm::mat4 view);

	void draw();

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
	
	// all GUI stuff
	VkDescriptorPool gui_descriptor_pool;

	//wrapper class for GLFWwindow
	std::shared_ptr<MyWindow> window;

	int current_frame = 0;

	// scene objects
	std::vector<MeshModel> model_list;
	// std::vector<Mesh> meshes;

	// scene settings
	struct UboViewProjection {
		glm::mat4 projection;
		glm::mat4 view;
	} ubo_view_projection;

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

	VkImage depth_buffer_image;
	VkDeviceMemory depth_buffer_image_memory;
	VkImageView depth_buffer_image_view;
	VkFormat depth_format;

	// - Descriptors
	VkDescriptorSetLayout descriptor_set_layout;								// for normal uniform values
	VkDescriptorSetLayout sampler_set_layout;									// descriptor set layout for our samplers
	VkPushConstantRange push_constant_range;

	VkDescriptorPool descriptor_pool;
	VkDescriptorPool sampler_descriptor_pool;
	std::vector<VkDescriptorSet> descriptor_sets;
	std::vector<VkDescriptorSet> sampler_descriptor_sets;				// these are no swap chain dependend descriptors, doesn't change over frames

	// for every model
	std::vector<VkBuffer> vp_uniform_buffer;
	std::vector<VkDeviceMemory> vp_uniform_buffer_memory;

	// changes between meshes  
	std::vector<VkBuffer> model_dynamic_uniform_buffer;
	std::vector<VkDeviceMemory> model_dynamic_uniform_buffer_memory;

	/*VkDeviceSize min_uniform_buffer_offset;
	size_t model_uniform_alignment;*/
	// Model* model_transfer_space;

	// assets
	VkSampler texture_sampler;
	std::vector<VkImage> texture_images;
	std::vector<VkDeviceMemory> texture_images_memory;
	std::vector<VkImageView> texture_image_views;

	// --PIPELINE --
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;

	// -- POOLS --
	VkCommandPool graphics_command_pool;

	// utilities
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;

	// -- SYNCHRONIZATION --
	std::vector<VkSemaphore> image_available;
	std::vector<VkSemaphore> render_finished;
	std::vector<VkFence> draw_fences;

	//Vulkan functions
	// all create functions
	void create_instance();
	void create_logical_device();
	void create_surface();
	void create_swap_chain();
	void create_render_pass();
	void create_descriptor_set_layout();
	void create_push_constant_range();
	void create_graphics_pipeline();
	void create_depthbuffer_image();
	void create_framebuffers();
	void create_command_pool();
	void create_command_buffers();
	void create_synchronization();
	void create_texture_sampler();

	void create_uniform_buffers();
	void create_descriptor_pool();
	void create_gui_context();
	void create_fonts_and_upload();
	void create_descriptor_sets();

	void update_uniform_buffers(uint32_t image_index);

	// - record functions
	void record_commands(uint32_t current_image);

	//get functions
	void get_physical_device();

	// allocate functions
	void allocate_dynamic_buffer_transfer_space();

	// support functions
	// checker functions
	bool check_instance_extension_support(std::vector<const char*>* check_extensions);
	bool check_device_extension_support(VkPhysicalDevice device);
	bool check_device_suitable(VkPhysicalDevice device);

	// getter functions
	QueueFamilyIndices get_queue_families(VkPhysicalDevice device);
	SwapChainDetails get_swapchain_details(VkPhysicalDevice device);

	//validation layers
	bool check_validation_layer_support();

	// choose functions
	VkSurfaceFormatKHR choose_best_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR choose_best_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities);
	VkFormat choose_supported_format(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags feature_flags);

	// create functions
	VkImage create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags use_flags,
											VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory);
	VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
	VkShaderModule create_shader_module(const std::vector<char>& code);

	int create_texture(std::string filename);
	int create_texture_image(std::string filename);
	int create_texture_descriptor(VkImageView texture_image);

	// loader functions
	stbi_uc* load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size);

};

