#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
#include <sstream>

#include "Allocator.h"
#include "Window.h"
#include "Utilities.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "ObjLoader.h"
#include "VulkanDevice.h"
#include "QueueFamilyIndices.h"
#include "VulkanSwapChain.h"
#include "VulkanInstance.h"
#include "GUISceneSharedVars.h"
#include "VulkanDebug.h"
#include "GlobalUBO.h"
#include "SceneUBO.h"
#include <PushConstantRasterizer.h>
#include <PushConstantRayTracing.h>
#include "VulkanBuffer.h"
#include "VulkanBufferManager.h"
#include "Texture.h"
#include "ASManager.h"
#include "MemoryHelper.h"
#include "CommandBufferManager.h"
#include "Scene.h"
#include <GUI.h>
#include "Rasterizer.h"
#include <PostStage.h>
#include <Raytracing.h>

#include "BottomLevelAccelerationStructure.h"
#include "TopLevelAccelerationStructure.h"

class VulkanRenderer
{
public:

	VulkanRenderer(	Window* window, 
					Scene*	scene,
					GUI*	gui,
					Camera* camera);

	void	hot_reload_all_shader();
	void	drawFrame(ImDrawData* gui_draw_data);

	void	update_uniforms(Scene* scene,
							Camera* camera,
							Window* window);

	void	updateStateDueToUserInput(GUI* gui);

	void	update_raytracing_descriptor_set(uint32_t image_index);
	void	record_commands(uint32_t image_index, ImDrawData* gui_draw_data);

	// texture functions 
	void	create_sampler_array_descriptor_set();

	void	clean_up_swapchain();
	void	clean_up_raytracing();
	void	clean_up();

	~VulkanRenderer();

private:

	GUIRendererSharedVars			guiRendererSharedVars;

	// helper class for managing our buffers
	VulkanBufferManager				vulkanBufferManager;

	// Vulkan instance, stores all per-application states
	VulkanInstance					instance;

	// surface defined on windows as WIN32 window system, Linux f.e. X11, MacOS also their own
	VkSurfaceKHR					surface;
	void							create_surface();

	std::unique_ptr<VulkanDevice>	device;

	VulkanSwapChain					vulkanSwapChain;
	void							recreate_swap_chain();

	Window*							window;
	Scene*							scene;

	// -- pools
	void							create_command_pool();
	void							cleanUpCommandPools();
	VkCommandPool					graphics_command_pool;
	VkCommandPool					compute_command_pool;

	// uniform buffers
	GlobalUBO						globalUBO;
	std::vector<VulkanBuffer>		globalUBOBuffer;
	SceneUBO						sceneUBO;
	std::vector<VulkanBuffer>		sceneUBOBuffer;
	void							create_uniform_buffers();
	void							update_uniform_buffers(uint32_t image_index);
	void							cleanUpUBOs();

	std::vector<VkCommandBuffer>	command_buffers;
	CommandBufferManager			commandBufferManager;
	void							create_command_buffers();

	Raytracing						raytracingStage;
	Rasterizer						rasterizer;
	PostStage						postStage;

	// new era of memory management for my project
	// for now on integrate vma 
	Allocator						allocator;

	// -- synchronization
	uint32_t						current_frame{ 0 };
	std::vector<VkSemaphore>		image_available;
	std::vector<VkSemaphore>		render_finished;
	std::vector<VkFence>			in_flight_fences;
	std::vector<VkFence>			images_in_flight_fences;
	void							create_synchronization();

	ASManager asManager;
	std::vector<BottomLevelAccelerationStructure>	blas;
	TopLevelAccelerationStructure					tlas;
	VulkanBuffer									objectDescriptionBuffer;

	void							create_descriptor_pool_object_description();
	void							create_object_description_buffer();

	VkSampler						texture_sampler;
	VkDescriptorPool				post_descriptor_pool{};
	VkDescriptorSetLayout			post_descriptor_set_layout;
	std::vector<VkDescriptorSet>	post_descriptor_set;
	void							create_post_descriptor();
	void							update_post_descriptor_set();

	void							createDescriptors();
	void							create_texture_sampler();
	void							create_descriptor_pool_uniforms();
	void							create_descriptor_pool_sampler();
	void							create_descriptor_sets();
	void							create_descriptor_set_layouts();
	VkDescriptorSetLayout			descriptor_set_layout;			
	VkDescriptorSetLayout			sampler_set_layout;				

	VkDescriptorPool				descriptor_pool;
	VkDescriptorPool				sampler_descriptor_pool;
	VkDescriptorPool				object_description_pool;
	std::vector<VkDescriptorSet>	descriptor_sets;
	VkDescriptorSet					sampler_descriptor_set;			

	VkDescriptorPool				raytracing_descriptor_pool;
	std::vector<VkDescriptorSet>	raytracing_descriptor_set;
	VkDescriptorSetLayout			raytracing_descriptor_set_layout;

	void							create_raytracing_descriptor_set_layouts();
	void							create_raytracing_descriptor_sets();
	void							create_raytracing_descriptor_pool();
	
	void							check_changed_framebuffer_size();
	bool							framebuffer_resized{false};

};

