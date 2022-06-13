#pragma once

#include <memory>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "CommandBufferManager.hpp"
#include "GUIRendererSharedVars.hpp"
#include "GUISceneSharedVars.hpp"
#include "Globals.hpp"
#include "VulkanDevice.hpp"
#include "Window.hpp"

class GUI {
 public:
  GUI(Window* window);

  void initializeVulkanContext(VulkanDevice* device, const VkInstance& instance,
                               const VkRenderPass& post_render_pass,
                               const VkCommandPool& graphics_command_pool);

  GUISceneSharedVars getGuiSceneSharedVars() { return guiSceneSharedVars; };
  GUIRendererSharedVars& getGuiRendererSharedVars() {
    return guiRendererSharedVars;
  };

  void render();

  void cleanUp();

  ~GUI();

 private:
  void create_gui_context(Window* window, const VkInstance& instance,
                          const VkRenderPass& post_render_pass);

  void create_fonts_and_upload(const VkCommandPool& graphics_command_pool);

  VulkanDevice* device{VK_NULL_HANDLE};
  Window* window{VK_NULL_HANDLE};
  VkDescriptorPool gui_descriptor_pool{VK_NULL_HANDLE};
  CommandBufferManager commandBufferManager;

  GUISceneSharedVars guiSceneSharedVars;
  GUIRendererSharedVars guiRendererSharedVars;
};
