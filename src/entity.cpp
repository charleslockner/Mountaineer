#include "entity.h"
#include "math.h"

Entity::Entity(Eigen::Vector3f position, Model * model, std::vector<EntityLimb> limbs) {
   this->position = position;
   this->rotation = Eigen::Quaternionf(1,0,0,0);
   this->scale = Eigen::Vector3f(1,1,1);
   this->model = model;

   for (int i = 0; i < MAX_BONES; i++) {
      this->boneTransforms[i] = Eigen::Matrix4f::Identity();
      this->animTransforms[i] = Eigen::Matrix4f::Identity();
   }

   this->boneController = new BoneController(model, boneTransforms, animTransforms, limbs);
}

Entity::~Entity() {
   delete boneController;
}

void Entity::update(float timeDelta) {
   boneController->setModelM(generateModelM());

   if (model->hasAnimations)
      boneController->updateTransforms(timeDelta);
}

Eigen::Matrix4f Entity::generateModelM() {
   return Mmath::TransformationMatrix(position, rotation, scale);
}
