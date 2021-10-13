#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

class Camera
{

public:

	Camera();

	Camera(glm::vec3 start_position, glm::vec3 start_up, float start_yaw, float start_pitch, 
					float start_move_speed, float start_turn_speed, 
					float near_plane, float far_plane, float fov);

	void key_control(bool* keys, GLfloat delta_time);
	void mouse_control(GLfloat x_change, GLfloat y_change);

	glm::vec3 get_camera_position();
	glm::vec3 get_camera_direction();
	glm::vec3 get_up_axis();
	glm::vec3 get_right_axis();
	GLfloat get_near_plane();
	GLfloat get_far_plane();
	GLfloat get_fov();
	GLfloat get_yaw();
	void set_near_plane(GLfloat near_plane);
	void set_far_plane(GLfloat far_plane);
	void set_fov(GLfloat fov);

	void set_camera_position(glm::vec3 new_camera_position);

	glm::mat4 calculate_viewmatrix();

	~Camera();

private:

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat movement_speed;
	GLfloat turn_speed;

	GLfloat near_plane, far_plane, fov;

	void update();
	float to_radians(float angle_in_degrees);
};

