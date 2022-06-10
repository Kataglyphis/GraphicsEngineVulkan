#include <SceneConfig.h>
#include "VulkanRendererConfig.h"

#include <filesystem>
//#define SULO_MODE 1

namespace sceneConfig {

std::string getModelFile() {
  std::stringstream modelFile;
  std::filesystem::path cwd = std::filesystem::current_path();
  modelFile << cwd.string();
  modelFile << RELATIVE_RESOURCE_PATH;

#if NDEBUG
  modelFile << "Model/crytek-sponza/";
  modelFile << "sponza_triag.obj";

#else
#ifdef SULO_MODE
  modelFile << "Model/Sulo/WolfStahl/";
  //modelFile << "Wolf-Stahl.obj";
  modelFile << "SuloLongDongLampe_v2.obj";
#else
  modelFile << "Model/VikingRoom/";
  modelFile << "viking_room.obj";
#endif
#endif

  return modelFile.str();
  // std::string modelFile =
  // "Model/crytek-sponza/sponza_triag.obj"; std::string modelFile
  // = "Model/Dinosaurs/dinosaurs.obj"; std::string modelFile =
  // "Model/Pillum/PilumPainting_export.obj"; std::string modelFile
  // = "Model/sibenik/sibenik.obj"; std::string modelFile =
  // "Model/sportsCar/sportsCar.obj"; std::string modelFile =
  // "Model/StanfordDragon/dragon.obj"; std::string modelFile =
  // "Model/CornellBox/CornellBox-Sphere.obj"; std::string
  // "Model/bunny/bunny.obj"; std::string modelFile =
  // "Model/buddha/buddha.obj"; std::string modelFile =
  // "Model/bmw/bmw.obj"; std::string modelFile =
  // "Model/testScene.obj"; std::string modelFile =
  // "Model/San_Miguel/san-miguel-low-poly.obj";
}

glm::mat4 getModelMatrix() {
  glm::mat4 modelMatrix(1.0f);

#if NDEBUG

  // dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f,
  // -50.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
  /*dragon_model = glm::rotate(dragon_model, glm::radians(-90.f),
     glm::vec3(1.0f, 0.0f, 0.0f)); dragon_model = glm::rotate(dragon_model,
     glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));*/

#else

// dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f,
// -50.0f));
#if SULO_MODE
  modelMatrix = glm::scale(modelMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
#else
  modelMatrix = glm::scale(modelMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
#endif

#endif

  return modelMatrix;
}

}  // namespace sceneConfig
