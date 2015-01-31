#include "entity.h"
#include "math.h"

Entity::Entity(glm::vec3 position, Model * model) {
   this->position = position;
   this->rotation = 0.0f;
   this->scale = glm::vec3(1.0f);
   this->model = model;

   for (int i = 0; i < model->boneCount; i++)
      this->boneTransforms[i] = glm::mat4(1.0f);
   for (int i = 0; i < model->boneCount; i++)
      this->boneRotations[i] = glm::quat(1, glm::vec3(0,0,0));

   this->boneController = new BoneController(model, boneTransforms, boneRotations);

   // this->animHandler = new AnimationHandler(model, boneTransforms);
   // this->animHandler->repeat(0);
}

Entity::~Entity() {
   delete boneController;
   // delete animHandler;
}

void Entity::draw(EntityShader * shader, float timeDelta) {
   boneController->rotateBone(3, 0.1, glm::vec3(1,0,0));
   // animHandler->updateTransforms(timeDelta);

   shader->sendEntityData(this);
   shader->render();
}

