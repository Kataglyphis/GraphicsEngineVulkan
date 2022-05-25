#pragma once
#include <memory>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Window.h"
#include "VulkanDevice.h"
#include "GUISceneSharedVars.h"
#include "GUIRendererSharedVars.h"
#include "CommandBufferManager.h"

class GUI
{
public:

	GUI(Window* window);

	void initializeVulkanContext(	VulkanDevice* device,
									const VkInstance& instance,
									const VkRenderPass& post_render_pass,
									const VkCommandPool& graphics_command_pool);

	const GUISceneSharedVars&		getGuiSceneSharedVars() { return guiSceneSharedVars; };
	GUIRendererSharedVars&			getGuiRendererSharedVars() { return guiRendererSharedVars; };

	ImDrawData*						render();



	~GUI();

private:

	void create_gui_context(Window* window, 
							const VkInstance& instance,
							const VkRenderPass& post_render_pass);

	void create_fonts_and_upload(const VkCommandPool& graphics_command_pool);

	VulkanDevice*			device;
	Window*					window;
	VkDescriptorPool		gui_descriptor_pool;
	CommandBufferManager	commandBufferManager;

	GUISceneSharedVars		guiSceneSharedVars;
	GUIRendererSharedVars	guiRendererSharedVars;
};

