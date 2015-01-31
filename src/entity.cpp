#include "entity.h"
#include "math.h"

Entity::Entity(glm::vec3 position, Model * model) {
   this->position = position;
   this->rotation = 0.0f;
   this->scale = glm::vec3(1.0f);
   this->model = model;
   for (int i = 0; i < model->boneCount; i++)
      this->boneTransforms[i] = glm::mat4(1.0f);

   this->animHandler = new AnimationHandler(model, boneTransforms);
   this->animHandler->repeat(0);
}

Entity::~Entity() {}

void Entity::draw(EntityShader * shader, float timeDelta) {
   animHandler->updateTransforms(timeDelta);

   shader->sendEntityData(this);
   shader->render();
}

