# define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "MyWindow.h"

#include "VulkanRenderer.h"

int main() {

    int window_width = 1200;
    int window_height = 768;

    std::shared_ptr<MyWindow> main_window = std::make_shared<MyWindow>(window_width, window_height);
    main_window->initialize();

    VulkanRenderer vulkan_renderer{};
    if (vulkan_renderer.init(main_window) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    while (!main_window->get_should_close()) {
    
        //poll all events incoming from user
        glfwPollEvents();

        main_window->swap_buffers();

    }

    return 0;

}

