#include "entity.h"
#include "math.h"

Entity::Entity(Eigen::Vector3f position, Model * model) {
   this->position = position;
   this->rotation = Eigen::Quaternionf(1,0,0,0);
   this->scale = Eigen::Vector3f(1,1,1);
   this->model = model;

   for (int i = 0; i < MAX_BONES; i++)
      this->boneTransforms[i] = Eigen::Matrix4f::Identity();

   this->boneController = new BoneController(model, boneTransforms);
   boneController->playAnimation(1, 0, true);
   // boneController->stopAnimation(0, true);
}

Entity::~Entity() {
   delete boneController;
}

void Entity::update(float timeDelta) {
   if (model->hasBoneTree) {
      // boneController->rotateBone(0, 0.003, Eigen::Vector3f(0,1,0));
      // boneController->rotateBone(4, 0.1, Eigen::Vector3f(0,1,0));
      // boneController->rotateBone(12, 0.5, Eigen::Vector3f(1,0,0));
      // boneController->rotateBone(15, 0.1, Eigen::Vector3f(0,0,1));
   }

   if (model->hasAnimations)
      boneController->updateTransforms(timeDelta);
}

Eigen::Matrix4f Entity::generateModelM() {
   return Mmath::transformationMatrix(position, rotation, scale);
}
