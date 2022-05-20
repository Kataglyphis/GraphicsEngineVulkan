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

#include "tiny_obj_loader.h"

#include "Scene.h"
#include <GUI.h>

class VulkanRenderer
{
public:

	VulkanRenderer(	Window* window, 
					Scene*	scene,
					GUI*	gui,
					Camera* camera);

	void	hot_reload_all_shader();
	int		create_model(std::string modelFile);

	void	update_uniforms(Scene* scene,
							Camera* camera,
							Window* window);

	void	updateStateDueToUserInput(GUI* gui);

	void update_raytracing_descriptor_set(uint32_t image_index);
	void record_commands(uint32_t image_index, ImDrawData* gui_draw_data);

	// texture functions 
	int create_texture(std::string filename);
	int create_texture_image(std::string filename);
	void create_sampler_array_descriptor_set();


	void drawFrame(ImDrawData* gui_draw_data);

	void clean_up_swapchain();
	void clean_up_raytracing();
	void clean_up();

	~VulkanRenderer();

private:

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
	VkCommandPool					graphics_command_pool;
	VkCommandPool					compute_command_pool;

	// uniforms
	GlobalUBO						globalUBO;
	SceneUBO						sceneUBO;

	PushConstantRasterizer			pc_raster;

	// uniform buffer
	std::vector<VkBuffer>		vp_uniform_buffer;
	std::vector<VkDeviceMemory> vp_uniform_buffer_memory;
	std::vector<VkBuffer>		directions_uniform_buffer;
	std::vector<VkDeviceMemory> directions_uniform_buffer_memory;

	std::vector<VkCommandBuffer>	command_buffers;
	std::vector<VkFramebuffer>		framebuffers;

	// new era of memory management for my project
	// for now on integrate vma 
	Allocator						allocator;

	// texture functions
	int create_texture_descriptor(VkImageView texture_image);

	// -- synchronization
	std::vector<VkSemaphore> image_available;
	std::vector<VkSemaphore> render_finished;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight_fences;
	//VkFence building_BLAS;
	void create_synchronization();

	// ----- VULKAN CORE COMPONENTS ----- END

	// ----- ALL RASTERIZER SPECIFICS ----- BEGIN 
	// -- pipeline
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;

	// -- Offscreen render pass
	VkPipeline offscreen_graphics_pipeline;
	VkPipelineLayout offscreen_pipeline_layout;
	VkRenderPass offscreen_render_pass;
	VkFormat offscreen_format{ VK_FORMAT_R32G32B32A32_SFLOAT };
	std::vector<VkFramebuffer> offscreen_framebuffer;
	std::vector<OffscreenTexture> offscreen_images;

	VkImage offscreen_depth_buffer_image;
	VkDeviceMemory offscreen_depth_buffer_image_memory;
	VkImageView offscreen_depth_buffer_image_view;
	//VkFormat offscreen_depth_format;

	void create_offscreen_graphics_pipeline();
	void create_offscreen_textures();
	void create_offscreen_render_pass();
	void create_offscreen_framebuffers();
	// -- Offscreen End 

	// -- Post 
	void create_post_renderpass();
	void create_post_pipeline();
	void create_post_descriptor();
	void update_post_descriptor_set();

	VkPushConstantRange post_push_constant_range;
	VkDescriptorPool post_descriptor_pool{};
	VkDescriptorSetLayout post_descriptor_set_layout;
	std::vector<VkDescriptorSet> post_descriptor_set;
	VkRenderPass post_render_pass;
	VkPipeline post_graphics_pipeline;
	VkPipelineLayout post_pipeline_layout;
	// -- Post - End

	// depth
	VkImage depth_buffer_image;
	VkDeviceMemory depth_buffer_image_memory;
	VkImageView depth_buffer_image_view;
	VkFormat depth_format;
	
	void init_rasterizer();
	// all create functions
	void create_render_pass();
	void create_descriptor_set_layouts();
	void create_push_constant_range();
	void create_rasterizer_graphics_pipeline();
	void create_depthbuffer_image();
	void create_framebuffers();

	void create_command_buffers();
	void create_texture_sampler();

	void create_uniform_buffers();
	void create_descriptor_pool_uniforms();
	void create_descriptor_pool_sampler();
	void create_descriptor_sets();

	

	// ----- ALL RASTERIZER SPECIFICS ----- END 

	// - descriptors
	VkDescriptorSetLayout descriptor_set_layout;	// for normal uniform values
	VkDescriptorSetLayout sampler_set_layout;		// descriptor set layout for our samplers
	VkPushConstantRange push_constant_range;
	VkDescriptorPool descriptor_pool;
	VkDescriptorPool sampler_descriptor_pool;
	VkDescriptorPool object_description_pool;
	std::vector<VkDescriptorSet> descriptor_sets;
	VkDescriptorSet sampler_descriptor_set;			// these are no swap chain dependend descriptors, doesn't change over frames

	// ----- ALL RAYTRACING SPECIFICS ----- BEGIN
	// -- en/-disable raytracing
	bool raytracing = false;
	void init_raytracing();
	// -- create funcs
	// -- bottom level acceleration structure
	void create_BLAS();
	void object_to_VkGeometryKHR(Mesh* mesh, VkAccelerationStructureGeometryKHR& acceleration_structure_geometry, 
								VkAccelerationStructureBuildRangeInfoKHR& acceleration_structure_build_range_info);

	void create_acceleration_structure_infos_BLAS(BuildAccelerationStructure& build_as_structure, BlasInput& blas_input,
													VkDeviceSize& current_scretch_size, VkDeviceSize& current_size);

	void create_single_blas(VkCommandBuffer command_buffer, BuildAccelerationStructure& build_as_structure, 
								VkDeviceAddress scratch_device_or_host_address);

	// -- top level acceleration structure
	void create_TLAS();
	void create_geometry_instance_buffer(VkBuffer& geometry_instance_buffer, VkDeviceMemory& geometry_instance_buffer_memory,
										std::vector<VkAccelerationStructureInstanceKHR> tlas_instances);

	void create_raytracing_pipeline();
	void create_shader_binding_table();

	void create_raytracing_descriptor_pool();

	void create_descriptor_pool_object_description();
	void create_object_description_buffer();
	void create_raytracing_descriptor_set_layouts();
	void create_raytracing_descriptor_sets();

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties{};

	// -- acceleration structure
	// -- bottom level
	std::vector<BLAS> blas;

	// -- top level
	TLAS tlas;

	// -- descriptors
	VkDescriptorPool raytracing_descriptor_pool;
	std::vector<VkDescriptorSet> raytracing_descriptor_set;
	VkDescriptorSetLayout raytracing_descriptor_set_layout;

	// -- pipeline
	VkPipeline raytracing_pipeline;
	VkPipelineLayout raytracing_pipeline_layout;

	// -- shader bindings
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	VkBuffer shader_binding_table_buffer;
	VkDeviceMemory shader_binding_table_buffer_memory;

	VkBuffer raygen_shader_binding_table_buffer;
	VkDeviceMemory raygen_shader_binding_table_buffer_memory;

	VkBuffer miss_shader_binding_table_buffer;
	VkDeviceMemory miss_shader_binding_table_buffer_memory;

	VkBuffer hit_shader_binding_table_buffer;
	VkDeviceMemory hit_shader_binding_table_buffer_memory;

	VkStridedDeviceAddressRegionKHR rgen_region{};
	VkStridedDeviceAddressRegionKHR miss_region{};
	VkStridedDeviceAddressRegionKHR hit_region{};
	VkStridedDeviceAddressRegionKHR call_region{};

	// -- properties
	// VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;

	PushConstantRaytracing pc_ray;
	VkPushConstantRange pc_ray_ranges;

	// ----- ALL RAYTRACING SPECIFICS ----- END

	// -- TEXTURE --
	VkSampler texture_sampler;
	std::vector<uint32_t> texture_mip_levels;

	VkBuffer object_description_buffer;
	VkDeviceMemory object_description_buffer_memory;

	std::vector<VkImage> texture_images;
	std::vector<VkDeviceMemory> texture_images_memory;
	std::vector<VkImageView> texture_image_views;
	// mipmapping
	int max_levels = 10;

	// -- UPDATE FUNCTIONS FOR THE DRAW COMMAND
	void update_uniform_buffers(uint32_t image_index);

	// ----- VARS ----- BEGIN
	void check_changed_framebuffer_size();
	bool framebuffer_resized;
	// indices index into current frame
	int current_frame;
	// ----- VARS ----- END

};

