#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(glm::vec3 start_position, glm::vec3 start_up, float start_yaw, float start_pitch,
								float start_move_speed, float start_turn_speed,
								float near_plane, float far_plane, float fov)
{

	position = start_position;
	world_up = start_up;
	yaw = start_yaw;
	pitch = start_pitch;
	//here we want the normal coord. axis z is showing to us !!
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	//choose it clever; is ordnial scale; so some trial and error in finding some good starting value
	movement_speed = start_move_speed;
	turn_speed = start_turn_speed;

	this->near_plane = near_plane;
	this->far_plane = far_plane;
	this->fov = fov;
}

void Camera::key_control(bool* keys, GLfloat delta_time)
{
	GLfloat velocity = movement_speed * delta_time;

	if (keys[GLFW_KEY_W]) {

		position += front * velocity;

	}

	if (keys[GLFW_KEY_D]) {

		position += right * velocity;

	}

	if (keys[GLFW_KEY_A]) {

		position += -right * velocity;

	}

	if (keys[GLFW_KEY_S]) {

		position += -front * velocity;

	}

	if (keys[GLFW_KEY_Q]) {

		yaw += -velocity;

	}

	if (keys[GLFW_KEY_E]) {

		yaw += velocity;

	}
}

void Camera::mouse_control(GLfloat x_change, GLfloat y_change)
{

	//here we only want to support views 90 degrees to each side 
	//again choose turn speed well in respect to its ordinal scale
	x_change *= turn_speed;
	y_change *= turn_speed;

	yaw += x_change;
	pitch += y_change;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}

	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	// by changing the rotations you need to update all parameters
	// for we retrieve them later for further calculations!
	update();

}

glm::vec3 Camera::get_camera_position()
{
	return position;
}

glm::vec3 Camera::get_camera_direction()
{
	return glm::normalize(front);
}

glm::vec3 Camera::get_up_axis()
{
	return up;
}

glm::vec3 Camera::get_right_axis()
{
	return right;
}

GLfloat Camera::get_near_plane()
{
	return near_plane;
}

GLfloat Camera::get_far_plane()
{
	return far_plane;
}

GLfloat Camera::get_fov()
{
	return fov;
}

GLfloat Camera::get_yaw()
{
	return yaw;
}

void Camera::set_near_plane(GLfloat near_plane)
{
	this->near_plane = near_plane;
}

void Camera::set_far_plane(GLfloat far_plane)
{
	this->far_plane = far_plane;
}

void Camera::set_fov(GLfloat fov)
{
	this->fov = fov;
}

void Camera::set_camera_position(glm::vec3 new_camera_position)
{
	this->position = new_camera_position;
}

glm::mat4 Camera::calculate_viewmatrix()
{
	//very necessary for further calc
	return glm::lookAt(position, position + front, up);
}

Camera::~Camera()
{
}

void Camera::update()
{
	//https://learnopengl.com/Getting-started/Camera?fbclid=IwAR1WEr4jt6IyWC52s_WKYHtaFoeug37pG5YqbDPifgn5F1UXPbUjWbJWiqQ
	// thats a bit tricky; have a look to link above if there a questions :)
	// but simple geometrical analysis
	// consider yaw you are turnig to the side; pich as you move the head forward and back; roll rotations around z-axis will make you dizzy :))
	// notice that to roll will not chnge my front vector
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	//retrieve the right vector with some world_up
	right = glm::normalize(glm::cross(front, world_up));

	// but this means the up vector must again be calculated with right vector calculated!!!
	up = glm::normalize(glm::cross(right, front));
}

float Camera::to_radians(float angle_in_degrees)
{
	return angle_in_degrees *(3.14159265f / 180.f);
}
