#include "entity.h"
#include "math.h"

#include "glm/gtx/rotate_vector.hpp"

Entity::Entity(glm::vec3 position, Model * model) {
   this->position = position;
   this->rotation = 0.0f;
   this->scale = glm::vec3(1.0f);
   this->model = model;

   for (int i = 0; i < MAX_BONES; i++)
      this->boneTransforms[i] = glm::mat4(1.0f);

   this->boneController = new BoneController(model, boneTransforms);
}

Entity::~Entity() {
   delete boneController;
}

void Entity::update(float timeDelta) {
   if (model->hasBoneTree) {
      boneController->rotateBone(13, -0.05, glm::normalize(glm::vec3(1,1,0)));
      boneController->rotateBone(3, 0.1, glm::normalize(glm::vec3(1,0,0)));
   }

   if (model->hasAnimations)
      boneController->updateTransforms(timeDelta);
}

glm::mat4 Entity::generateModelM() {
   glm::mat4 transM = glm::translate(glm::mat4(1.0f), position);
   glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0,1,0));
   glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), scale);
   return transM * rotateM * scaleM;
}