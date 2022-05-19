#include "App.h"

#define STB_IMAGE_IMPLEMENTATION

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "Window.h"

#include "VulkanRenderer.hpp"
#include "GUI.h"

App::App()
{
}

int App::run()
{

    int window_width = 1200;
    int window_height = 768;

    float angle = 0.0f;
    float delta_time = 0.0f;
    float last_time = 0.0f;

    // -- RAY TRACING ON
    bool raytracing = false;

    std::unique_ptr<Window> main_window     = std::make_unique<Window>(window_width, window_height);
    std::unique_ptr<Scene>  initial_scene   = std::make_unique<Scene>();
    std::unique_ptr<GUI>    gui             = std::make_unique<GUI>(main_window.get());
    std::unique_ptr<Camera> camera          = std::make_unique<Camera>();

    VulkanRenderer vulkan_renderer{ main_window.get(),
                                    initial_scene.get(),
                                    gui.get(),
                                    camera.get(),
                                    raytracing };

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
        camera->key_control(main_window->get_keys(), delta_time);
        camera->mouse_control(main_window->get_x_change(), main_window->get_y_change());

        vulkan_renderer.update_view(camera->calculate_viewmatrix());
        vulkan_renderer.update_view_direction(camera->get_camera_direction());
        vulkan_renderer.update_cam_pos(camera->get_camera_position());

        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;

        angle += 10.f * delta_time;
        if (angle > 360.f) {
            angle = 0.0f;
        }

        // retrieve updates from the UI
        bool shader_hot_reload_triggered = false;
        ImDrawData* gui_draw_data = gui->render(shader_hot_reload_triggered, raytracing);
        if (shader_hot_reload_triggered) vulkan_renderer.hot_reload_all_shader();
        //glm::mat4 dragon_model(1.0f);
        ////dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
        //dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
        //dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        //vulkan_renderer.update_model(0, dragon_model);
        //vulkan_renderer.update_model(1, floor_model);


        vulkan_renderer.drawFrame(gui_draw_data);

    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

App::~App()
{
}
