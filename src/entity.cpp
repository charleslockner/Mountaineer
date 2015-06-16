#include "entity.h"
#include "matrix_math.h"
#include "animation.h"

#include <assert.h>
#include <math.h>

// -------------------------------------------------------- //
// ========================= Entity ======================= //
// -------------------------------------------------------- //
Entity::Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot)
: position(pos), rotation(rot) {}
Entity::Entity(Eigen::Vector3f pos)
: position(pos), rotation(Eigen::Quaternionf(1,0,0,0)) {}

Entity::~Entity() {}

// direction should be normalized
void Entity::moveAlong(Eigen::Vector3f direction, float distance) {
   position += distance * direction;
}

void Entity::moveLeft(float dist) {
   moveAlong(getLeft(), dist);
}

void Entity::moveRight(float dist) {
   moveAlong(-getLeft(), dist);
}

void Entity::moveForward(float dist) {
   moveAlong(getForward(), dist);
}

void Entity::moveBackward(float dist) {
   moveAlong(-getForward(), dist);
}

void Entity::moveUp(float dist) {
   moveAlong(getUp(), dist);
}

void Entity::moveDown(float dist) {
   moveAlong(-getUp(), dist);
}

void Entity::rotateAlong(float angle, Eigen::Vector3f axis) {
   rotation *= Eigen::Quaternionf(Eigen::AngleAxisf(angle, axis));
}

void Entity::lookAt(Eigen::Vector3f target) {
   Eigen::Vector3f dir = (target - position).normalized();
   rotation = Eigen::Quaternionf::FromTwoVectors(FORWARD_BASE, dir);
}

Eigen::Vector3f Entity::getLeft() {
   return (rotation._transformVector(LEFT_BASE)).normalized();
}

Eigen::Vector3f Entity::getUp() {
   return (rotation._transformVector(UP_BASE)).normalized();
}

Eigen::Vector3f Entity::getForward() {
   return (rotation._transformVector(FORWARD_BASE)).normalized();
}

// --------------------------------------------------------- //
// ===================== Model Entity ====================== //
// --------------------------------------------------------- //
ModelEntity::ModelEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: Entity(pos, rot), scale(scl), model(model) {}
ModelEntity::ModelEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: Entity(pos, rot), scale(Eigen::Vector3f(1,1,1)), model(model) {}
ModelEntity::ModelEntity(Eigen::Vector3f pos, Model * model)
: Entity(pos), scale(Eigen::Vector3f(1,1,1)), model(model) {}

Eigen::Matrix4f ModelEntity::generateModelM() {
   return Mmath::TransformationMatrix(position, rotation, scale);
}

// --------------------------------------------------------- //
// ==================== Animated Entity ==================== //
// --------------------------------------------------------- //
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: ModelEntity(pos, rot, scl, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: ModelEntity(pos, rot, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Model * model)
: ModelEntity(pos, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::~AnimatedEntity() {}

// --------------------------------------------------------- //
// ====================== Mocap Entity ===================== //
// --------------------------------------------------------- //
MocapEntity::MocapEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: AnimatedEntity(pos, rot, scl, model),
  animNum(0), animTime(0), animIsPlaying(false) {}
MocapEntity::MocapEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: AnimatedEntity(pos, rot, model),
  animNum(0), animTime(0), animIsPlaying(false) {}
MocapEntity::MocapEntity(Eigen::Vector3f pos, Model * model)
: AnimatedEntity(pos, model),
  animNum(0), animTime(0), animIsPlaying(false) {}

void MocapEntity::playAnimation(int animNum) {
   animIsPlaying = true;
   this->animNum = animNum;
}

void MocapEntity::stopAnimation() {
   animIsPlaying = false;
}

void MocapEntity::update(float tickDelta) {
   if (model->hasAnimations) {
      // Move forward the animation time
      float duration = model->animations[animNum].duration;
      animTime += tickDelta;
      if (animTime > duration)
         animTime -= duration;

      // Compute each bone's (even though there's no heirarchy) animation transform
      for (int boneIndex = 0; boneIndex < model->boneCount; boneIndex++) {
         Bone * bone = & model->bones[boneIndex];
         Animation * anim = & model->animations[this->animNum];
         AnimBone * animBone = & anim->animBones[boneIndex];

         Eigen::Matrix4f animKeysM = AN::ComputeKeyframeTransform(animBone, anim->keyCount, this->animTime, anim->duration);
         animMs[boneIndex] = animKeysM * bone->invBonePose;
      }
   }
}

// --------------------------------------------------------- //
// ==================== Bonified Entity =================== //
// --------------------------------------------------------- //
BonifiedEntity::BonifiedEntity(Eigen::Vector3f pos, Model * model)
: AnimatedEntity(pos, model) {

   this->boneMs = std::vector<Eigen::Matrix4f>(model->boneCount);
   this->animNums = std::vector<int>(model->boneCount);
   this->bonesPlaying = std::vector<bool>(model->boneCount);
   this->animTimes = std::vector<float>(model->boneCount);

   for (int i = 0; i < model->boneCount; i++) {
      this->boneMs[i] = Eigen::Matrix4f::Identity();
      this->animNums[i] = 0;
      this->bonesPlaying[i] = false;
      this->animTimes[i] = 0;
   }
}

void BonifiedEntity::playAnimation(int animNum) {
   playAnimation(animNum, model->boneRoot, true);
}

void BonifiedEntity::playAnimation(int animNum, int boneNum, bool isRecursive) {
   assert(animNum < model->animationCount);
   this->animNums[boneNum] = animNum;
   this->bonesPlaying[boneNum] = true;

   if (isRecursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         playAnimation(animNum, model->bones[boneNum].childIndices[i], isRecursive);
}

void BonifiedEntity::stopAnimation() {
   stopAnimation(model->boneRoot, true);
}

void BonifiedEntity::stopAnimation(int boneNum, bool isRecursive) {
   this->bonesPlaying[boneNum] = false;

   if (isRecursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         stopAnimation(model->bones[boneNum].childIndices[i], isRecursive);
}

void BonifiedEntity::update(float tickDelta) {
   if (model->hasAnimations && model->hasBoneTree) {
      // Start replaying animation if finished
      replayIfNeeded(tickDelta);
      // Recursively fill in the animMs
      computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
   }
}

void BonifiedEntity::replayIfNeeded(float tickDelta) {
   for (int i = 0; i < model->boneCount; i++) {
      if (!model->hasBoneTree || bonesPlaying[i]) {
         float duration = model->animations[animNums[i]].duration;

         animTimes[i] += tickDelta;
         if (animTimes[i] > duration)
            animTimes[i] -= duration;
      }
   }
}

void BonifiedEntity::computeAnimMs(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = animNums[boneIndex];
   float tickTime = animTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   Eigen::Matrix4f keyframeM = AN::ComputeKeyframeTransform(
      animBone, anim->keyCount, tickTime, anim->duration);

   boneMs[boneIndex] = parentM * keyframeM;
   animMs[boneIndex] = boneMs[boneIndex] * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeAnimMs(bone->childIndices[i], boneMs[boneIndex]);
}
