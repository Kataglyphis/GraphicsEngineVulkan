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

#include "stb_image.h"

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
#include "tiny_obj_loader.h"

#include "Scene.h"
#include <GUI.h>

class VulkanRenderer
{
public:

	VulkanRenderer(	Window* window, 
					Scene*	scene,
					GUI*	gui,
					Camera* camera,
					bool raytracing);

	int create_model(std::string modelFile);

	void update_model(int model_id, glm::mat4 new_model);
	void update_view(glm::mat4 view);
	void update_light_direction(glm::vec3 light_dir);
	void update_raytracing(bool raytracing_on);
	void update_view_direction(glm::vec3 view_dir);
	void update_cam_pos(glm::vec3 cam_pos);

	void update_raytracing_descriptor_set(uint32_t image_index);
	void record_commands(uint32_t image_index, ImDrawData* gui_draw_data);

	// texture functions 
	int create_texture(std::string filename);
	int create_texture_image(std::string filename);
	void create_sampler_array_descriptor_set();

	void hot_reload_all_shader();

	void drawFrame(ImDrawData* gui_draw_data);

	void clean_up_swapchain();
	void clean_up_raytracing();
	void clean_up();

	~VulkanRenderer();

private:

	// Vulkan instance, stores all per-application states
	VkInstance						instance;
	// surface defined on windows as WIN32 window system, Linux f.e. X11, MacOS also their own
	VkSurfaceKHR					surface;
	std::unique_ptr<VulkanDevice>	device;

	VulkanSwapChain					vulkanSwapChain;

	Window*							window;
	Scene*							scene;

	// -- pools
	VkCommandPool					graphics_command_pool;
	VkCommandPool					compute_command_pool;

	std::vector<VkCommandBuffer>	command_buffers;
	std::vector<VkFramebuffer>		framebuffers;

	// all regarding swapchain
	/*VkSwapchainKHR swapchain;
	std::vector<SwapChainImage> swap_chain_images;
	std::vector<VkFramebuffer> swap_chain_framebuffers;
	std::vector<VkCommandBuffer> command_buffers;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;*/

	// new era of memory management for my project
	// for now on integrate vma 
	// Allocator allocator;

	void recreate_swap_chain();

	// core create functions
	void create_instance();
	void create_vma_allocator();
	void create_surface();

	// -- support functions 
	// helper create functions
	//VkImage create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags use_flags,
	//											VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory);
	//VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);

	VkShaderModule create_shader_module(const std::vector<char>& code);

	// texture functions
	int create_texture_descriptor(VkImageView texture_image);

	// checker functions
	bool check_instance_extension_support(std::vector<const char*>* check_extensions);

	// -- debugging
	VkDebugUtilsMessengerEXT debug_messenger;
	bool check_validation_layer_support();

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
	void create_command_pool();
	void create_command_buffers();
	void create_texture_sampler();

	void create_uniform_buffers();
	void create_descriptor_pool_uniforms();
	void create_descriptor_pool_sampler();
	void create_descriptor_sets();

	// uniforms
	UboViewProjection ubo_view_projection{};
	UboDirections ubo_directions{};
	PushConstantRaster pc_raster;

	// uniform buffer
	std::vector<VkBuffer> vp_uniform_buffer;
	std::vector<VkDeviceMemory> vp_uniform_buffer_memory;
	std::vector<VkBuffer> directions_uniform_buffer;
	std::vector<VkDeviceMemory> directions_uniform_buffer_memory;

	// ----- ALL RASTERIZER SPECIFICS ----- END 

	// - descriptors
	VkDescriptorSetLayout descriptor_set_layout;								// for normal uniform values
	VkDescriptorSetLayout sampler_set_layout;									// descriptor set layout for our samplers
	VkPushConstantRange push_constant_range;
	VkDescriptorPool descriptor_pool;
	VkDescriptorPool sampler_descriptor_pool;
	VkDescriptorPool object_description_pool;
	std::vector<VkDescriptorSet> descriptor_sets;
	VkDescriptorSet sampler_descriptor_set;				// these are no swap chain dependend descriptors, doesn't change over frames

	// ----- ALL RAYTRACING SPECIFICS ----- BEGIN
	// -- en/-disable raytracing
	bool raytracing;
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

	// ---- HELPER ---- BEGIN
	stbi_uc* load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size);
	// ---- HELPER ---- END

	// -- UPDATE FUNCTIONS FOR THE DRAW COMMAND
	void update_uniform_buffers(uint32_t image_index);

	// ----- VARS ----- BEGIN
	
	void check_changed_framebuffer_size();
	bool framebuffer_resized;
	// indices index into current frame
	int current_frame;
	// ----- VARS ----- END

};

