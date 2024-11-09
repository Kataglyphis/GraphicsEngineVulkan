#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
 public:
  Camera();

  Camera(glm::vec3 start_position, glm::vec3 start_up, GLfloat start_yaw,
         GLfloat start_pitch, GLfloat start_move_speed,
         GLfloat start_turn_speed, GLfloat near_plane, GLfloat far_plane,
         GLfloat fov);

  void key_control(bool* keys, GLfloat delta_time);
  void mouse_control(GLfloat x_change, GLfloat y_change);

  glm::vec3 get_camera_position() const { return position; };
  glm::vec3 get_camera_direction() const { return glm::normalize(front); };
  glm::vec3 get_up_axis() const { return up; };
  glm::vec3 get_right_axis() const { return right; };
  GLfloat get_near_plane() const { return near_plane; };
  GLfloat get_far_plane() const { return far_plane; };
  GLfloat get_fov() const { return fov; };
  GLfloat get_yaw() const { return yaw; };
  glm::mat4 get_viewmatrix() const;

  void set_near_plane(GLfloat near_plane);
  void set_far_plane(GLfloat far_plane);
  void set_fov(GLfloat fov);
  void set_camera_position(glm::vec3 new_camera_position);

  ~Camera();

 private:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 world_up;
  glm::vec3 right;
  glm::vec3 up;

  GLfloat yaw;
  GLfloat pitch;

  GLfloat movement_speed;
  GLfloat turn_speed;

  GLfloat near_plane, far_plane, fov;

  void update();
};
