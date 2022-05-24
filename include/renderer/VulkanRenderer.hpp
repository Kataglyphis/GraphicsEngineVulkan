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

#include "Scene.h"
#include <GUI.h>

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
	std::vector<VkFramebuffer>		framebuffers;

	// new era of memory management for my project
	// for now on integrate vma 
	Allocator						allocator;

	// -- synchronization
	uint32_t					current_frame{ 0 };
	std::vector<VkSemaphore>	image_available;
	std::vector<VkSemaphore>	render_finished;
	std::vector<VkFence>		in_flight_fences;
	std::vector<VkFence>		images_in_flight_fences;
	void						create_synchronization();

	// ----- VULKAN CORE COMPONENTS ----- END

	// -- Offscreen render pass
	VkPipeline					offscreen_graphics_pipeline;
	VkPipelineLayout			offscreen_pipeline_layout;
	VkRenderPass				offscreen_render_pass;
	VkFormat					offscreen_format{ VK_FORMAT_R32G32B32A32_SFLOAT };
	std::vector<VkFramebuffer>	offscreen_framebuffer;
	std::vector<Texture>		offscreen_images;
	Texture						offscreenDepthBuffer;
	void						init_offscreen();
	void						create_offscreen_graphics_pipeline();
	void						create_offscreen_textures();
	void						create_offscreen_render_pass();
	void						create_offscreen_framebuffers();
	// -- Offscreen End 

	// -- Post 
	VkPushConstantRange				post_push_constant_range;
	VkDescriptorPool				post_descriptor_pool{};
	VkDescriptorSetLayout			post_descriptor_set_layout;
	std::vector<VkDescriptorSet>	post_descriptor_set;
	VkRenderPass					post_render_pass;
	VkPipeline						post_graphics_pipeline;
	VkPipelineLayout				post_pipeline_layout;
	// texture sampler for everything ---- change that ! 
	VkSampler						texture_sampler;
	void							init_post();
	void							create_post_renderpass();
	void							create_post_pipeline();
	void							create_post_descriptor();
	void							update_post_descriptor_set();
	// -- Post - End

	// ----- ALL RASTERIZER SPECIFICS ----- BEGIN 
	PushConstantRasterizer	pc_raster;
	VkPipeline				graphics_pipeline;
	VkPipelineLayout		pipeline_layout;
	VkRenderPass			render_pass;
	Texture					depthBufferImage;
	VkFormat				depth_format;
	
	void					init_rasterizer();
	void					create_render_pass();
	void					create_descriptor_set_layouts();
	void					create_push_constant_range();
	void					create_rasterizer_graphics_pipeline();
	void					create_depthbuffer_image();
	void					create_framebuffers();

	void					create_command_buffers();
	void					create_texture_sampler();

	void					create_descriptor_pool_uniforms();
	void					create_descriptor_pool_sampler();
	void					create_descriptor_sets();
	// ----- ALL RASTERIZER SPECIFICS ----- END 

	// - descriptors
	VkDescriptorSetLayout			descriptor_set_layout;			// for normal uniform values
	VkDescriptorSetLayout			sampler_set_layout;				// descriptor set layout for our samplers
	VkPushConstantRange				push_constant_range;
	VkDescriptorPool				descriptor_pool;
	VkDescriptorPool				sampler_descriptor_pool;
	VkDescriptorPool				object_description_pool;
	std::vector<VkDescriptorSet>	descriptor_sets;
	VkDescriptorSet					sampler_descriptor_set;			// these are no swap chain dependend descriptors, doesn't change over frames

	// ----- ALL RAYTRACING SPECIFICS ----- BEGIN
	// -- en/-disable raytracing
	bool							raytracing{ false };
	VkDescriptorPool				raytracing_descriptor_pool;
	std::vector<VkDescriptorSet>	raytracing_descriptor_set;
	VkDescriptorSetLayout			raytracing_descriptor_set_layout;
	VkPipeline						raytracing_pipeline;
	VkPipelineLayout				raytracing_pipeline_layout;
	PushConstantRaytracing			pc_ray;
	VkPushConstantRange				pc_ray_ranges;
	void							init_raytracing();
	void							create_raytracing_descriptor_set_layouts();
	void							create_raytracing_descriptor_sets();
	void							create_raytracing_pipeline();
	void							create_shader_binding_table();
	void							create_raytracing_descriptor_pool();

	// -- shader bindings
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	VulkanBuffer					shaderBindingTableBuffer;
	VulkanBuffer					raygenShaderBindingTableBuffer;
	VulkanBuffer					missShaderBindingTableBuffer;
	VulkanBuffer					hitShaderBindingTableBuffer;

	VkStridedDeviceAddressRegionKHR rgen_region{};
	VkStridedDeviceAddressRegionKHR miss_region{};
	VkStridedDeviceAddressRegionKHR hit_region{};
	VkStridedDeviceAddressRegionKHR call_region{};


	VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties{};
	// ----- ALL RAYTRACING SPECIFICS ----- END
	

	// ----- ALL ACCELERATION STRUCTURES ----- BEGIN
	std::vector<BottomLevelAccelerationStructure>	blas;
	TopLevelAccelerationStructure					tlas;
	VulkanBuffer									objectDescriptionBuffer;

	void create_BLAS();
	void object_to_VkGeometryKHR(	Mesh* mesh, 
									VkAccelerationStructureGeometryKHR& acceleration_structure_geometry, 
									VkAccelerationStructureBuildRangeInfoKHR& acceleration_structure_build_range_info);

	void create_acceleration_structure_infos_BLAS(	BuildAccelerationStructure& build_as_structure, 
													BlasInput& blas_input,
													VkDeviceSize& current_scretch_size, 
													VkDeviceSize& current_size);

	void create_single_blas(VkCommandBuffer command_buffer, 
							BuildAccelerationStructure& build_as_structure,
							VkDeviceAddress scratch_device_or_host_address);

	void create_TLAS();

	void create_descriptor_pool_object_description();
	void create_object_description_buffer();

	void check_changed_framebuffer_size();
	bool framebuffer_resized{false};

};

