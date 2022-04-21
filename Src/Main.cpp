#define STB_IMAGE_IMPLEMENTATION

#define GLFW_INCLUDE_VULKAN
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

#include "VulkanRenderer.hpp"

int main() {

    // Setup Vulkan
    /*if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");

        return EXIT_FAILURE;
    }*/

    int window_width = 1200;
    int window_height = 768;

    std::shared_ptr<MyWindow> main_window = std::make_shared<MyWindow>(window_width, window_height);
    main_window->initialize();

    glm::vec3 start_position = glm::vec3(0.0f, 100.0f, -80.0f);
    float near_plane = 0.1f;
    float far_plane = 4000.f;

    float angle = 0.0f;
    float delta_time = 0.0f;
    float last_time = 0.0f;
    float fov = 60.f;

    glm::vec3 start_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float start_yaw = 80.f;
    float start_pitch = -40.0f;
    float start_move_speed = 200.f;
    float start_turn_speed = 0.25f;

    // -- RAY TRACING ON
    bool raytracing = false;

    std::shared_ptr<Scene> initial_scene = std::make_shared<Scene>();

    Camera camera{ start_position, start_up, start_yaw, start_pitch,
                                    start_move_speed, start_turn_speed,
                                    near_plane, far_plane, fov };

    VulkanRenderer vulkan_renderer{};

    if (vulkan_renderer.init(main_window, initial_scene, start_position, near_plane,
                                            far_plane, fov, camera.get_camera_direction(),
                                            raytracing) == EXIT_FAILURE) {
        
        return EXIT_FAILURE;

    }

    glm::mat4 dragon_model(1.0f);
    //dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
    //dragon_model = glm::scale(dragon_model, glm::vec3(10.0f, 10.0f, 10.0f));
    /*dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
    dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));*/
    vulkan_renderer.update_model(0, dragon_model);
    vulkan_renderer.update_raytracing(raytracing);

    // ----- !!!IMPORTANT!!! we initialize raytracin after setting up the scene
    // ----- we are building the acceleration structures from the scene and the scene must NOT be EMPTY!!!!
    while (!main_window->get_should_close()) {
    
        //poll all events incoming from user
        glfwPollEvents();

        // handle events for the camera
        camera.key_control(main_window->get_keys(), delta_time);
        camera.mouse_control(main_window->get_x_change(), main_window->get_y_change());

        vulkan_renderer.update_view(camera.calculate_viewmatrix());
        vulkan_renderer.update_view_direction(camera.get_camera_direction());
        vulkan_renderer.update_cam_pos(camera.get_camera_position());

        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;

        angle += 10.f * delta_time;
        if (angle > 360.f) {
            angle = 0.0f;
        }

        //glm::mat4 dragon_model(1.0f);
        ////dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
        //dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
        //dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        //vulkan_renderer.update_model(0, dragon_model);
        //vulkan_renderer.update_model(1, floor_model);


        vulkan_renderer.drawFrame();

    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

