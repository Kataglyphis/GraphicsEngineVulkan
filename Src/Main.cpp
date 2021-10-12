#define STB_IMAGE_IMPLEMENTATION
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

    float angle = 0.0f;
    float delta_time = 0.0f;
    float last_time = 0.0f;

    while (!main_window->get_should_close()) {
    
        //poll all events incoming from user
        glfwPollEvents();

        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;

        angle += 10.f * delta_time;
        if (angle > 360.f) {
            angle = 0.0f;
        }

        glm::mat4 first_model(1.0f);
        glm::mat4 second_model(1.0f);

        first_model = glm::translate(first_model, glm::vec3(-2.0f, 0.0f, -5.0f));
        first_model = glm::rotate(first_model, glm::radians(angle), glm::vec3(0.0f,0.0f,1.0f));

        second_model = glm::translate(second_model, glm::vec3(2.0f, 0.0f, -5.0f));
        second_model = glm::rotate(second_model, glm::radians(-angle*100), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan_renderer.update_model(0, first_model);
        vulkan_renderer.update_model(1, second_model);

        vulkan_renderer.draw();
        //main_window->swap_buffers();

    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

