#pragma once
#include <stdio.h>

#include <GLFW/glfw3.h>

class MyWindow
{

public:

	MyWindow();
	MyWindow(GLint window_width, GLint window_height);

	// init glfw and its context ...
	int initialize();

	// GETTER functions
	bool get_should_close() { return glfwWindowShouldClose(main_window); }
	GLfloat get_buffer_width() { return (GLfloat)window_buffer_width; }
	GLfloat get_buffer_height() { return (GLfloat)window_buffer_height; }
	GLfloat get_x_change();
	GLfloat get_y_change();
	GLFWwindow* get_window() {return main_window;}
	bool* get_keys() { return keys; }
	bool framebuffer_size_has_changed();
	void reset_framebuffer_has_changed();

	// SETTER functions
	void update_viewport();
	void set_buffer_size(GLfloat window_buffer_width, GLfloat window_buffer_height);

	~MyWindow();

private:

	GLFWwindow* main_window;
	GLint window_width, window_height;
	// what key(-s) was/were pressed
	bool keys[1024];
	GLfloat last_x;
	GLfloat last_y;
	GLfloat x_change;
	GLfloat y_change;
	bool mouse_first_moved;
	bool framebuffer_resized;

	//buffers to store our window data to
	GLint window_buffer_width, window_buffer_height;

	//we need to start our window callbacks for interaction
	void init_callbacks();
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	
	//need to be static ...
	static void key_callback(GLFWwindow* window, int key, int code, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	
};

