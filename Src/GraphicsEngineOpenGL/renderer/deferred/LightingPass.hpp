#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "Camera.hpp"
#include "GBuffer.hpp"
#include "LightingPassShaderProgram.hpp"
//#include "Noise.hpp"

#include "RenderPass.hpp"
#include "Quad.hpp"
//#include "RandomNumbers.hpp"
#include "Scene.hpp"

class LightingPass : public RenderPass {
 public:
  LightingPass();

  void execute(glm::mat4 projection_matrix, std::shared_ptr<Camera>,
               std::shared_ptr<Scene> scene, std::shared_ptr<GBuffer> gbuffer,
               float delta_time);

  void create_shader_program();

  ~LightingPass();

 private:
  glm::vec3 current_offset;

  void set_uniforms(glm::mat4 projection_matrix,
                    std::shared_ptr<Camera> main_camera,
                    std::shared_ptr<Scene> scene,
                    std::shared_ptr<GBuffer> gbuffer, float delta_time);

  std::shared_ptr<LightingPassShaderProgram> shader_program;

  Quad quad;
};
