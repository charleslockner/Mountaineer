#include "entity.h"

#include "animation.h"
#include "math.h"

Entity::Entity(glm::vec3 position, Model * model) {
   this->position = position;
   this->rotation = 0.0f;
   this->scale = glm::vec3(1.0f);

   for (int i = 0; i < model->boneCount; i++)
      this->boneTransforms[i] = glm::mat4(1.0f);

   this->model = model;
}

Entity::~Entity() {}

void Entity::draw(EntityShader * shader, float time) {
   float animTime = fmod(time, model->animations[0].duration);
   // printf("animTime = %f\n", animTime);
   computeBoneTransforms(boneTransforms, model, 0, animTime);

   shader->sendEntityData(this);
   shader->render();
}

