#include "entity.h"
#include "math.h"

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

void Entity::draw(EntityShader * shader, float timeDelta) {
   if (model->hasBones) {
      // boneController->rotateBone(13, -0.05, glm::normalize(glm::vec3(1,1,0)));
      boneController->rotateBone(3, 0.1, glm::normalize(glm::vec3(1,0,0)));
      boneController->updateTransforms(timeDelta);
   }

   shader->renderEntity(this);
}