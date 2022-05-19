#include "Window.h"

Window::Window() :  
                    
                    window_width(800.f),
                    window_height(600.f),
                    x_change(0.0f),
                    y_change(0.0f), 
                    framebuffer_resized(false)

{

    // all keys non-pressed in the beginning
    for (size_t i = 0; i < 1024; i++) {
        keys[i] = 0;
    }

    initialize();

}

//please use this constructor; never the standard
Window::Window(GLint window_width, GLint window_height) : 
                        
                        framebuffer_resized(false), 
                        window_width(window_width),  
                        window_height(window_height),
                        x_change(0.0f),
                        y_change(0.0f)

{

    // all keys non-pressed in the beginning
    for (size_t i = 0; i < 1024; i++) {
        keys[i] = 0;
    }

    initialize();

}

int Window::initialize() {
    
    if (!glfwInit()) {

        printf("GLFW Init failed!");
        glfwTerminate();
        return 1;

    }

    // setup glfw window properties
    
    //should work with vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //lets work with nothing older than version 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    //allow it to resize
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    //retrieve new window
    main_window = glfwCreateWindow(window_width, window_height, "\\__/ Epic graphics from hell \\__/ ", NULL, NULL);

    if (!main_window) {

        printf("GLFW Window creation failed!");
        glfwTerminate();
        return 1;

    }

    // get buffer size information
    glfwGetFramebufferSize(main_window, &window_buffer_width, &window_buffer_height);

    // set context for GLEW to use
    glfwMakeContextCurrent(main_window);

    //Handle key + mouse Input
    //TODO: here we need to further go on and implement game logic
    init_callbacks();
    glfwSetInputMode(main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glEnable(GL_DEPTH_TEST);

    // setup viewport size
    // glViewport(0, 0, window_buffer_width, window_buffer_height);

    glfwSetWindowUserPointer(main_window, this);

    //you need to register function for handling changing window sizes
    glfwSetFramebufferSizeCallback(main_window, framebuffer_size_callback);

    return 0;
}

void Window::update_viewport() {

    glfwGetFramebufferSize(main_window, &window_buffer_width, &window_buffer_height);
    // setup viewport size
    //glViewport(0, 0, window_buffer_width, window_buffer_height);

}

void Window::set_buffer_size(GLfloat window_buffer_width, GLfloat window_buffer_height) {
    this->window_buffer_width = window_buffer_width;
    this->window_buffer_height = window_buffer_height;
}

GLfloat Window::get_x_change()
{
    GLfloat the_change = x_change;
    x_change = 0.0f;
    return the_change;
}

GLfloat Window::get_y_change()
{
    GLfloat the_change = y_change;
    y_change = 0.0f;
    return the_change;
}

GLfloat Window::get_height()
{
    return GLfloat(window_height);
}

GLfloat Window::get_width()
{
    return GLfloat(window_width);
}

bool Window::framebuffer_size_has_changed()
{
    return framebuffer_resized;
}


void Window::init_callbacks()
{
    //TODO: remember this section for our later game logic 
    //for the space ship to fly around
    glfwSetKeyCallback(main_window, key_callback);
    glfwSetMouseButtonCallback(main_window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(main_window, framebuffer_size_callback);

}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->framebuffer_resized = true;
    app->window_width = width;
    app->window_height = height;

}

void Window::reset_framebuffer_has_changed()
{
    this->framebuffer_resized = false;

}

void Window::key_callback(GLFWwindow* window, int key, int code, int action, int mode)
{
    Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            the_window->keys[key] = true;

        }
        else if (action == GLFW_RELEASE) {
            the_window->keys[key] = false;

        }
    }
}

void Window::mouse_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

    // need to handle first occurance of a mouse moving event
    if (the_window->mouse_first_moved) {
        the_window->last_x = (float)x_pos;
        the_window->last_y = (float)y_pos;
        the_window->mouse_first_moved = false;
    }

    the_window->x_change = (float)(x_pos - the_window->last_x);
    // take care of correct substraction :)
    the_window->y_change = (float)(the_window->last_y - y_pos);

    //update params
    the_window->last_x = (float)x_pos;
    the_window->last_y = (float)y_pos;
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if ((action == GLFW_PRESS) && (button == GLFW_MOUSE_BUTTON_RIGHT)) {
        glfwSetCursorPosCallback(window, mouse_callback);
    }
    else {
        the_window->mouse_first_moved = true;
        glfwSetCursorPosCallback(window, NULL);
    }
}

Window::~Window()
{
    glfwDestroyWindow(main_window);
    glfwTerminate();
}