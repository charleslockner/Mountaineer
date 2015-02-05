#include "entity.h"
#include "math.h"

Entity::Entity(glm::vec3 position, Model * model) {
   this->position = position;
   this->rotation = 0.0f;
   this->scale = glm::vec3(1.0f);
   this->model = model;

   for (int i = 0; i < model->boneCount; i++)
      this->boneTransforms[i] = glm::mat4(1.0f);

   this->boneController = new BoneController(model, boneTransforms);
}

Entity::~Entity() {
   delete boneController;
}

void Entity::draw(EntityShader * shader, float timeDelta) {
   // boneController->rotateBone(13, -0.05, glm::normalize(glm::vec3(1,1,0)));
   // boneController->rotateBone(9, 0.1, glm::normalize(glm::vec3(1,0,1)));
   boneController->updateTransforms(timeDelta);

   shader->sendEntityData(this);
   shader->render();
}