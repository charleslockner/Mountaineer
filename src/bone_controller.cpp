
#include "bone_controller.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include <stdio.h>
#include <assert.h>
#include <math.h>

static glm::vec3 lerpVec3(glm::vec3 begin, glm::vec3 end, float ratio) {
   return begin + ratio * (end - begin);
}

// should improve this to constant tickTime somehow (instead of log(keyCount))
static int findEarlyKeyIndex(Key * keys, int keyCount, float tickTime) {
   assert(keyCount >= 1);
   if (keyCount <= 2)
      return 0;

   int halfCount = keyCount / 2;

   if (tickTime < keys[halfCount].time)
      return findEarlyKeyIndex(keys, halfCount + 1, tickTime);
   else
      return halfCount + findEarlyKeyIndex(keys + halfCount, keyCount - halfCount, tickTime);
}

static Key interpolateKeys(Key earlyKey, Key lateKey, float ratio) {
   Key key;
   key.position = lerpVec3(earlyKey.position, lateKey.position, ratio);
   key.rotation = glm::slerp(earlyKey.rotation, lateKey.rotation, ratio);
   key.scale = lerpVec3(earlyKey.scale, lateKey.scale, ratio);
   return key;
}

static glm::mat4 computeAnimTransform(AnimBone * animBone, int keyCount, float tickTime) {
   Key * keys = animBone->keys;

   int earlyNdx = findEarlyKeyIndex(animBone->keys, keyCount, tickTime);
   int lateNdx = earlyNdx + 1;

   Key earlyKey = keys[earlyNdx];
   Key lateKey = keys[lateNdx];
   Key interpKey;

   if (keyCount == 1)
      interpKey = earlyKey;
   else {
      float ratio = (tickTime - earlyKey.time) / (lateKey.time - earlyKey.time);
      interpKey = interpolateKeys(earlyKey, lateKey, ratio);
   }

   glm::mat4 transM = glm::translate(glm::mat4(1.0), interpKey.position);
   glm::mat4 rotateM = glm::toMat4(interpKey.rotation);
   glm::mat4 scaleM = glm::scale(glm::mat4(1.0), interpKey.scale);

   return transM * rotateM * scaleM;
}

BoneController::BoneController(Model * model, glm::mat4 * boneTransforms) {
   this->model = model;
   this->boneTransforms = boneTransforms;

   for (int i = 0; i < model->boneCount; i++) {
      this->boneRotations[i] = glm::quat(1, glm::vec3(0,0,0));
      this->boneAnimNums[i] = 0;
      this->boneTimes[i] = 0;
      this->bonePlaying[i] = true;
   }
}

BoneController::~BoneController() {}

void BoneController::rotateBone(int boneIndex, float angle, glm::vec3 axis) {
   glm::quat rotQuat = glm::angleAxis(angle, axis);
   boneRotations[boneIndex] = boneRotations[boneIndex] * rotQuat;
}

void BoneController::playAnimation(int boneNum, int animNum) {
   boneAnimNums[boneNum] = animNum;
   boneTimes[boneNum] = 0;
   bonePlaying[boneNum] = true;
   assert(animNum < model->animationCount);
}

void BoneController::stopAnimation(int boneNum) {
   bonePlaying[boneNum] = false;
}

void BoneController::updateTransforms(float tickDelta) {
   for (int i = 0; i < model->boneCount; i++) {
      if (bonePlaying[i]) {
         float duration = model->animations[boneAnimNums[i]].duration;

         boneTimes[i] += tickDelta;
         if (boneTimes[i] > duration)
            boneTimes[i] -= duration;
      }
   }

   // recursively fill in bone transforms
   computeBoneTransform(model->boneRoot, glm::mat4(1.0f));
}

void BoneController::computeBoneTransform(int boneIndex, glm::mat4 parentM) {
   int animNum = boneAnimNums[boneIndex];
   float tickTime = boneTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   Animation * animation = & model->animations[animNum];
   AnimBone * animBone = & animation->animBones[boneIndex];
   int keyCount = animation->keyCount;

   glm::mat4 animKeysM = computeAnimTransform(animBone, keyCount, tickTime);
   glm::mat4 animPoseM = bone->parentOffset;
   glm::mat4 bonePoseM = bone->invBonePose;
   glm::mat4 rotationM = glm::toMat4(boneRotations[boneIndex]);

   // glm::mat4 animM = parentM * animPoseM * rotationM;
   glm::mat4 animM = parentM * animKeysM * rotationM;
   // glm::mat4 animM = parentM * animPoseM * animKeysM * rotationM; // use this for bind-space keys
   boneTransforms[boneIndex] = animM * bonePoseM;

   for (int i = 0; i < bone->childCount; i++)
      computeBoneTransform(bone->childIndices[i], animM);
}
