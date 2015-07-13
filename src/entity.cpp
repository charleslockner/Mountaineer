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
   moveAlong(getRight(), dist);
}
void Entity::moveForward(float dist) {
   moveAlong(getForward(), dist);
}
void Entity::moveBackward(float dist) {
   moveAlong(getBackward(), dist);
}
void Entity::moveUp(float dist) {
   moveAlong(getUp(), dist);
}
void Entity::moveDown(float dist) {
   moveAlong(getDown(), dist);
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
Eigen::Vector3f Entity::getRight() {
   return (rotation._transformVector(RIGHT_BASE)).normalized();
}
Eigen::Vector3f Entity::getUp() {
   return (rotation._transformVector(UP_BASE)).normalized();
}
Eigen::Vector3f Entity::getDown() {
   return (rotation._transformVector(DOWN_BASE)).normalized();
}
Eigen::Vector3f Entity::getForward() {
   return (rotation._transformVector(FORWARD_BASE)).normalized();
}
Eigen::Vector3f Entity::getBackward() {
   return (rotation._transformVector(BACKWARD_BASE)).normalized();
}

// --------------------------------------------------------- //
// ==================== Static Entity ====================== //
// --------------------------------------------------------- //
StaticEntity::StaticEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: Entity(pos, rot), scale(scl), model(model) {
   initializePhysics();
}
StaticEntity::StaticEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: Entity(pos, rot), scale(Eigen::Vector3f(1,1,1)), model(model) {
   initializePhysics();
}
StaticEntity::StaticEntity(Eigen::Vector3f pos, Model * model)
: Entity(pos), scale(Eigen::Vector3f(1,1,1)), model(model) {
   initializePhysics();
}

void StaticEntity::initializePhysics() {
   linearMomentum = Eigen::Vector3f(0,0,0);
   angularMomentum = Eigen::Vector3f(0,0,0);

   force = Eigen::Vector3f(0,0,0);
   torque = Eigen::Vector3f(0,0,0);
}

void StaticEntity::applyForce(Eigen::Vector3f f) {
   force += f;
}

void StaticEntity::applyTorque(Eigen::Vector3f t) {
   torque += t;
}

void StaticEntity::physicsStep(float timeDelta) {
   // Update the linear and angular momentums
   linearMomentum  += timeDelta * force;
   angularMomentum += timeDelta * torque;

   // Update the translation
   Eigen::Vector3f velocity = linearMomentum / model->mass;
   position += timeDelta * velocity;

   // Update the orientation
   Eigen::Matrix3f invInertiaWorld = rotation * model->invInertiaTensor * rotation.conjugate();
   Eigen::Vector3f omega = invInertiaWorld * angularMomentum;
   Eigen::Quaternionf omegaRotation = Eigen::Quaternionf(0.0f, omega(0), omega(1), omega(2)) * rotation;
   rotation.w() += 0.5f * timeDelta * omegaRotation.w();
   rotation.x() += 0.5f * timeDelta * omegaRotation.x();
   rotation.y() += 0.5f * timeDelta * omegaRotation.y();
   rotation.z() += 0.5f * timeDelta * omegaRotation.z();
   rotation.normalize();

   force = Eigen::Vector3f(0,0,0);
   torque = Eigen::Vector3f(0,0,0);
}

float StaticEntity::getLinearEnergy() {
   Eigen::Vector3f velocity = linearMomentum / model->mass;
   return 0.5f * model->mass * velocity.transpose() * velocity;
}

float StaticEntity::getRotationalEnergy() {
   Eigen::Matrix3f invInertiaWorld = rotation * model->invInertiaTensor * rotation.conjugate();
   Eigen::Vector3f omega = invInertiaWorld * angularMomentum;
   return 0.5f * omega.transpose() * (rotation * model->inertiaTensor * rotation.conjugate()) * omega;
}

Eigen::Matrix4f StaticEntity::generateModelM() {
   return Mmath::TransformationMatrix(position, rotation, scale);
}

// --------------------------------------------------------- //
// ==================== Animated Entity ==================== //
// --------------------------------------------------------- //
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: StaticEntity(pos, rot, scl, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: StaticEntity(pos, rot, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Model * model)
: StaticEntity(pos, model) {
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

      // Compute each bone's animation transform (even though there's no bone heirarchy)
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
// ===================== Skinned Entity ==================== //
// --------------------------------------------------------- //
SkinnedEntity::SkinnedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: AnimatedEntity(pos, rot, scl, model) {
   initialize();
}
SkinnedEntity::SkinnedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: AnimatedEntity(pos, rot, model) {
   initialize();
}
SkinnedEntity::SkinnedEntity(Eigen::Vector3f pos, Model * model)
: AnimatedEntity(pos, model) {
   initialize();
}

void SkinnedEntity::initialize() {
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

void SkinnedEntity::playAnimation(int animNum) {
   playAnimation(animNum, model->boneRoot, true);
}

void SkinnedEntity::playAnimation(int animNum, int boneNum, bool isRecursive) {
   assert(animNum < model->animationCount);
   this->animNums[boneNum] = animNum;
   this->bonesPlaying[boneNum] = true;

   if (isRecursive)
      for (int i = 0; i < model->bones[boneNum].childIndices.size(); i++)
         playAnimation(animNum, model->bones[boneNum].childIndices[i], isRecursive);
}

void SkinnedEntity::stopAnimation() {
   stopAnimation(model->boneRoot, true);
}

void SkinnedEntity::stopAnimation(int boneNum, bool isRecursive) {
   this->bonesPlaying[boneNum] = false;

   if (isRecursive)
      for (int i = 0; i < model->bones[boneNum].childIndices.size(); i++)
         stopAnimation(model->bones[boneNum].childIndices[i], isRecursive);
}

void SkinnedEntity::update(float tickDelta) {
   if (model->hasAnimations && model->hasBoneTree) {
      // Start replaying animation if finished
      replayIfNeeded(tickDelta);
      // Recursively fill in the animMs
      computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
   }
}

void SkinnedEntity::replayIfNeeded(float tickDelta) {
   for (int i = 0; i < model->boneCount; i++) {
      if (!model->hasBoneTree || bonesPlaying[i]) {
         float duration = model->animations[animNums[i]].duration;

         animTimes[i] += tickDelta;
         if (animTimes[i] > duration)
            animTimes[i] -= duration;
      }
   }
}

void SkinnedEntity::computeAnimMs(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = animNums[boneIndex];
   float tickTime = animTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   Eigen::Matrix4f keyframeM = AN::ComputeKeyframeTransform(
      animBone, anim->keyCount, tickTime, anim->duration);

   boneMs[boneIndex] = parentM * keyframeM;
   animMs[boneIndex] = boneMs[boneIndex] * bone->invBonePose;

   for (int i = 0; i < bone->childIndices.size(); i++)
      computeAnimMs(bone->childIndices[i], boneMs[boneIndex]);
}
