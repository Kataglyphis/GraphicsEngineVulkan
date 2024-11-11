#pragma once
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include "ShaderProgram.hpp"
//#include "host_device_shared.hpp"

class ComputeShaderProgram : public ShaderProgram {
 public:
  ComputeShaderProgram();

  void reload();

  ~ComputeShaderProgram();

 private:
};
