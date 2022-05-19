#pragma once
#include <memory>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Window.h"
#include "Scene.h"
#include "VulkanDevice.h"

class GUI
{
public:

	GUI(Window* window);

	void initializeVulkanContext(	VulkanDevice* device,
									const VkInstance& instance,
									const VkRenderPass& post_render_pass,
									const VkCommandPool& graphics_command_pool);

	ImDrawData* render(bool& shader_hot_reload_triggered,
				bool& raytracing);

	void update_user_input(std::shared_ptr<Scene> scene);

	~GUI();

private:

	void create_gui_context(Window* window, 
							const VkInstance& instance,
							const VkRenderPass& post_render_pass);

	void create_fonts_and_upload(const VkCommandPool& graphics_command_pool);

	VulkanDevice*		device;
	Window*				window;
	VkDescriptorPool	gui_descriptor_pool;

	bool no_context_created = false;

	float direcional_light_ambient_intensity	= 10.f;
	float directional_light_color[3]			= { 1.f,1.f,1.f };
	float directional_light_direction[3]		= { 0.075f,-1.f,0.118f };

};

