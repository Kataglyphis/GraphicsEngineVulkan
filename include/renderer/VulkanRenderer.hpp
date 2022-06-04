#pragma once
#define GLFW_INCLUDE_NONE
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
#include "VulkanDevice.h"
#include "QueueFamilyIndices.h"
#include "VulkanSwapChain.h"
#include "VulkanInstance.h"
#include "GUISceneSharedVars.h"
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
#include <PathTracing.h>

class VulkanRenderer
{
public:

	VulkanRenderer(	Window* window, 
					Scene*	scene,
					GUI*	gui,
					Camera* camera);

	void	drawFrame();

	void	updateUniforms(	Scene* scene,
							Camera* camera,
							Window* window);

	void	updateStateDueToUserInput(GUI* gui);
	void	finishAllRenderCommands();
	void	update_raytracing_descriptor_set(uint32_t image_index);

	void	cleanUp();

	~VulkanRenderer();

private:

	void							shaderHotReload();

	// helper class for managing our buffers
	VulkanBufferManager				vulkanBufferManager;

	// Vulkan instance, stores all per-application states
	VulkanInstance					instance;

	// surface defined on windows as WIN32 window system, Linux f.e. X11, MacOS also their own
	VkSurfaceKHR					surface;
	void							create_surface();

	std::unique_ptr<VulkanDevice>	device;

	VulkanSwapChain					vulkanSwapChain;

	Window*							window;
	Scene*							scene;
	GUI*							gui;

	// -- pools
	void							record_commands(uint32_t image_index);
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
	PathTracing						pathTracing;
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
	void							createSynchronization();
	void							cleanUpSync();

	ASManager						asManager;
	VulkanBuffer					objectDescriptionBuffer;
	void							create_object_description_buffer();

	VkDescriptorPool				descriptorPoolSharedRenderStages;
	void							createDescriptorPoolSharedRenderStages();
	VkDescriptorSetLayout			sharedRenderDescriptorSetLayout;
	void							createSharedRenderDescriptorSetLayouts();
	std::vector<VkDescriptorSet>	sharedRenderDescriptorSet;
	void							createSharedRenderDescriptorSet();
	void							updateTexturesInSharedRenderDescriptorSet();

	VkDescriptorPool				post_descriptor_pool{ VK_NULL_HANDLE };
	VkDescriptorSetLayout			post_descriptor_set_layout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet>	post_descriptor_set;
	void							create_post_descriptor_layout();
	void							updatePostDescriptorSets();

	VkDescriptorPool				raytracingDescriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet>	raytracingDescriptorSet;
	VkDescriptorSetLayout			raytracingDescriptorSetLayout{ VK_NULL_HANDLE };

	void							createRaytracingDescriptorSetLayouts();
	void							createRaytracingDescriptorSets();
	void							updateRaytracingDescriptorSets();
	void							createRaytracingDescriptorPool();
	
	bool							checkChangedFramebufferSize();

};

