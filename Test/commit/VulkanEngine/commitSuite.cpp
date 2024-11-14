#include <gtest/gtest.h>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "GUI.hpp"
#include "VulkanRenderer.hpp"
#include "Window.hpp"


// Demonstrate some basic assertions.
TEST(HelloTestCommit, BasicAssertions)
{

    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(Integration, VulkanEngine)
{
  EXPECT_EQ(7 * 6, 42);
	int window_width = 1200;
  int window_height = 768;

  float delta_time = 0.0f;
  float last_time = 0.0f;

  std::unique_ptr<Window> window =
      std::make_unique<Window>(window_width, window_height);
  std::unique_ptr<Scene> scene = std::make_unique<Scene>();
  std::unique_ptr<GUI> gui = std::make_unique<GUI>(window.get());
  std::unique_ptr<Camera> camera = std::make_unique<Camera>();

  VulkanRenderer vulkan_renderer{window.get(), scene.get(), gui.get(),
                                  camera.get()};
}