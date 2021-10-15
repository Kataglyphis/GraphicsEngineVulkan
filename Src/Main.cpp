#define STB_IMAGE_IMPLEMENTATION
# define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

// all IMGUI stuff
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include "MyWindow.h"

#include "VulkanRenderer.h"

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
    float far_plane = 500.f;


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    VulkanRenderer vulkan_renderer{};
    if (vulkan_renderer.init(main_window, start_position, near_plane, far_plane) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    float angle = 0.0f;
    float delta_time = 0.0f;
    float last_time = 0.0f;
    float fov = 60.f;

    glm::vec3 start_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float start_yaw = 80.f;
    float start_pitch = -40.0f;
    float start_move_speed = 75.f;
    float start_turn_speed = 0.25f;

    // GUI variables
    float direcional_light_ambient_intensity = 10.f;
    float directional_light_color[3] = {1.f,1.f,1.f};
    float directional_light_direction[3] = {1.f,1.f,1.f};

    int gui_logo = vulkan_renderer.create_texture("Engine_logo.png");

    Camera camera{ start_position, start_up, start_yaw, start_pitch,
                                    start_move_speed, start_turn_speed,
                                    near_plane, far_plane, fov };

    //int dragon = vulkan_renderer.create_mesh_model("../Resources/Model/Dragon 2.5_3ds.3ds");
    int dragon = vulkan_renderer.create_mesh_model("../Resources/Model/Dragon 2.5_fbx.fbx");
    int floor = vulkan_renderer.create_mesh_model("../Resources/Model/Photoscan - Koeln_Drecksfeld_01.obj");

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
        //dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
        dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
        dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 floor_model(1.0f);
        floor_model = glm::scale(floor_model, glm::vec3(70.f));
        floor_model = glm::rotate(floor_model, glm::radians(130.f), glm::vec3(1.0f, 0.0f, 0.0f));
        floor_model = glm::translate(floor_model, glm::vec3(0.0f, 0.0f, -3.75f));
        //floor_model = glm::rotate(floor_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan_renderer.update_model(dragon, dragon_model);
        vulkan_renderer.update_model(floor, floor_model);

        /*glm::mat4 first_model(1.0f);
        glm::mat4 second_model(1.0f);

        first_model = glm::translate(first_model, glm::vec3(-2.0f, 0.0f, -5.0f));
        first_model = glm::rotate(first_model, glm::radians(angle), glm::vec3(0.0f,0.0f,1.0f));

        second_model = glm::translate(second_model, glm::vec3(2.0f, 0.0f, -5.0f));
        second_model = glm::rotate(second_model, glm::radians(-angle*100), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan_renderer.update_model(0, first_model);
        vulkan_renderer.update_model(1, second_model);*/

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();

        // render your GUI
        ImGui::Begin("GUI v1.0");

        if (ImGui::CollapsingHeader("Hot shader reload")) {

            ImGui::Text("Upcoming :)");

        }

        ImGui::Separator();


        if (ImGui::CollapsingHeader("Graphic Settings")) {

            if (ImGui::TreeNode("Directional Light")) {
                ImGui::Separator();
                ImGui::SliderFloat("Ambient intensity", &direcional_light_ambient_intensity, 0.0f, 50.0f);
                ImGui::Separator();
                // Edit a color (stored as ~4 floats)
                ImGui::ColorEdit3("Directional Light Color", directional_light_color);
                ImGui::Separator();
                ImGui::SliderFloat3("Light Direction", directional_light_direction, -1.f, 1.0f);

                ImGui::TreePop();
            }

        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("GUI Settings")) {

            ImGuiStyle& style = ImGui::GetStyle();

            if (ImGui::SliderFloat("Frame Rounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f")) {
                style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
            }
            { bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("KEY Bindings")) {

            ImGui::Text("WASD for moving Forward, backward and to the side\n QE for rotating ");

        }

        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //ImGui::ShowDemoWindow();
        // (void*)(intptr_t)&
        VkDescriptorSet ds = vulkan_renderer.get_texture_descriptor_set(gui_logo);
        ImGui::Image((void*)ds, ImVec2(300, 150));

        ImGui::End();

        // Rendering

        ImGui::Render();

        vulkan_renderer.draw();


    }

    vulkan_renderer.clean_up();

    return EXIT_SUCCESS;

}

