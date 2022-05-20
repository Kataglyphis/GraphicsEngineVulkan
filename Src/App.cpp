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

    float delta_time = 0.0f;
    float last_time = 0.0f;

    // -- RAY TRACING ON
    bool raytracing = false;

    std::unique_ptr<Window> window          = std::make_unique<Window>(window_width, window_height);
    std::unique_ptr<Scene>  scene           = std::make_unique<Scene>();
    std::unique_ptr<GUI>    gui             = std::make_unique<GUI>(window.get());
    std::unique_ptr<Camera> camera          = std::make_unique<Camera>();

    VulkanRenderer vulkan_renderer{ window.get(),
                                    scene.get(),
                                    gui.get(),
                                    camera.get(),
                                    raytracing };

    glm::mat4 dragon_model(1.0f);
    //dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
    //dragon_model = glm::scale(dragon_model, glm::vec3(10.0f, 10.0f, 10.0f));
    //dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));

    vulkan_renderer.update_model(0, dragon_model);
    vulkan_renderer.update_raytracing(raytracing);

    while (!window->get_should_close()) {

        //poll all events incoming from user
        glfwPollEvents();

        // handle events for the camera
        camera->key_control(window->get_keys(), delta_time);
        camera->mouse_control(window->get_x_change(), window->get_y_change());


        float now = static_cast<float>(glfwGetTime());
        delta_time = now - last_time;
        last_time = now;


        scene->update_user_input(gui.get());

        vulkan_renderer.update_uniforms(scene.get(),
                                        camera.get());

        // retrieve updates from the UI
        bool shader_hot_reload_triggered = false;
        ImDrawData* gui_draw_data = gui->render(shader_hot_reload_triggered, raytracing);
        if (shader_hot_reload_triggered) vulkan_renderer.hot_reload_all_shader();

        vulkan_renderer.drawFrame(gui_draw_data);

    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

App::~App()
{
}
