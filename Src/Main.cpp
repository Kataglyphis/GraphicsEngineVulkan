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

    glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 0.0f);
    VulkanRenderer vulkan_renderer{};
    if (vulkan_renderer.init(main_window, start_position) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    float angle = 0.0f;
    float delta_time = 0.0f;
    float last_time = 0.0f;
    float near_plane = 0.1f;
    float far_plane = 100.f;
    float fov = 60.f;

    glm::vec3 start_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float start_yaw = -60.f;
    float start_pitch = 0.0f;
    float start_move_speed = 75.f;
    float start_turn_speed = 0.25f;

    Camera camera{ start_position, start_up, start_yaw, start_pitch,
                                    start_move_speed, start_turn_speed,
                                    near_plane, far_plane, fov };

    //int dragon = vulkan_renderer.create_mesh_model("../Resources/Model/Dragon 2.5_3ds.3ds");
    int dragon = vulkan_renderer.create_mesh_model("../Resources/Model/Dragon 2.5_fbx.fbx");

    while (!main_window->get_should_close()) {
    
        //poll all events incoming from user
        glfwPollEvents();

        // handle events for the camera
        camera.key_control(main_window->get_keys(), delta_time);
        camera.mouse_control(main_window->get_x_change(), main_window->get_y_change());

        vulkan_renderer.update_view(camera.calculate_viewmatrix());

        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;

        angle += 10.f * delta_time;
        if (angle > 360.f) {
            angle = 0.0f;
        }
        
        glm::mat4 dragon_model(1.0f);
        dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -70.0f));
        dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        vulkan_renderer.update_model(dragon, dragon_model);

        /*glm::mat4 first_model(1.0f);
        glm::mat4 second_model(1.0f);

        first_model = glm::translate(first_model, glm::vec3(-2.0f, 0.0f, -5.0f));
        first_model = glm::rotate(first_model, glm::radians(angle), glm::vec3(0.0f,0.0f,1.0f));

        second_model = glm::translate(second_model, glm::vec3(2.0f, 0.0f, -5.0f));
        second_model = glm::rotate(second_model, glm::radians(-angle*100), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan_renderer.update_model(0, first_model);
        vulkan_renderer.update_model(1, second_model);*/

        vulkan_renderer.draw();
        //main_window->swap_buffers();

    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

