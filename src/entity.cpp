#include "entity.h"
#include "math.h"

Entity::Entity(Eigen::Vector3f position, Model * model) {
   this->position = position;
   this->rotation = Eigen::Quaternionf(1,0,0,0);
   this->scale = Eigen::Vector3f(1,1,1);
   this->model = model;

   for (int i = 0; i < MAX_BONES; i++)
      this->boneTransforms[i] = Eigen::Matrix4f::Identity();

   // this->boneController = new BoneController(model, boneTransforms);
}

Entity::~Entity() {
   delete boneController;
}

void Entity::update(float timeDelta) {
   // if (model->hasBoneTree) {
   //    // boneController->rotateBone(13, -0.05, glm::normalize(glm::vec3(1,1,0)));
   //    // boneController->rotateBone(2, 0.1, glm::normalize(glm::vec3(1,0,0)));
   // }

   // if (model->hasAnimations)
   //    boneController->updateTransforms(timeDelta);
}

Eigen::Matrix4f Entity::generateModelM() {
   Eigen::Matrix4f transM = makeTranslationMatrix(position);
   Eigen::Matrix4f rotateM = makeRotationMatrix(rotation);
   Eigen::Matrix4f scaleM = makeScaleMatrix(scale);
   // return transM * rotateM * scaleM;
   return Eigen::Matrix4f::Identity();
}
