#include <SceneConfig.h>

//#define SULO_MODE 0

namespace sceneConfig {

std::string getModelFile() {
  std::stringstream modelFile;
  modelFile << CMAKELISTS_DIR;
#if NDEBUG
  modelFile << "/Resources/Model/crytek-sponza/";
  modelFile << "sponza_triag.obj";

#else
#ifdef SULO_MODE
  modelFile << "/Resources/Model/Sulo/";
  modelFile << "SuloLongDongLampe_v2.obj";
#else
  modelFile << "/Resources/Model/VikingRoom/";
  modelFile << "viking_room.obj";
#endif
#endif

  return modelFile.str();
  // std::string modelFile =
  // "../Resources/Model/crytek-sponza/sponza_triag.obj"; std::string modelFile
  // = "../Resources/Model/Dinosaurs/dinosaurs.obj"; std::string modelFile =
  // "../Resources/Model/Pillum/PilumPainting_export.obj"; std::string modelFile
  // = "../Resources/Model/sibenik/sibenik.obj"; std::string modelFile =
  // "../Resources/Model/sportsCar/sportsCar.obj"; std::string modelFile =
  // "../Resources/Model/StanfordDragon/dragon.obj"; std::string modelFile =
  // "../Resources/Model/CornellBox/CornellBox-Sphere.obj"; std::string
  // modelFile = "../Resources/Model/bunny/bunny.obj"; std::string modelFile =
  // "../Resources/Model/buddha/buddha.obj"; std::string modelFile =
  // "../Resources/Model/bmw/bmw.obj"; std::string modelFile =
  // "../Resources/Model/testScene.obj"; std::string modelFile =
  // "../Resources/Model/San_Miguel/san-miguel-low-poly.obj";
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
