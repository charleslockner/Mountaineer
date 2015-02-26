
#include "bone_controller.h"
#include <Eigen/Dense>

#include <stdio.h>
#include <assert.h>
#include <math.h>

static Eigen::Vector3f lerpVec3(Eigen::Vector3f begin, Eigen::Vector3f end, float ratio) {
   return begin + ratio * (end - begin);
}

static int findEarlyKeyIndex(int keyCount, float tickTime, float duration) {
   int index = tickTime * (keyCount-1) / duration;
   return index < keyCount-1 ? index : keyCount - 2;
}

static Key interpolateKeys(Key earlyKey, Key lateKey, float tickTime) {
   Key key;
   tickTime = fmax(earlyKey.time, fmin(lateKey.time, tickTime));
   float ratio = (tickTime - earlyKey.time) / (lateKey.time - earlyKey.time);

   key.position = lerpVec3(earlyKey.position, lateKey.position, ratio);
   key.rotation = earlyKey.rotation.slerp(ratio, lateKey.rotation);
   key.scale = lerpVec3(earlyKey.scale, lateKey.scale, ratio);
   return key;
}

static Eigen::Matrix4f computeAnimTransform(AnimBone * animBone, int keyCount, float tickTime, float duration) {
   Key * keys = animBone->keys;
   assert(keyCount >= 1);

   int earlyNdx = findEarlyKeyIndex(keyCount, tickTime, duration);
   int lateNdx = earlyNdx + 1;

   Key interpKey = (keyCount == 1) ? keys[earlyNdx] :
      interpolateKeys(keys[earlyNdx], keys[lateNdx], tickTime);

   return makeTransformationMatrix(interpKey.position, interpKey.rotation, interpKey.scale);
}

BoneController::BoneController(Model * model, Eigen::Matrix4f * boneTransforms) {
   this->model = model;
   this->boneTransforms = boneTransforms;

   for (int i = 0; i < model->boneCount; i++) {
      this->boneRotations[i] = Eigen::Quaternionf(1,0,0,0); // May need to put the 1 last
      this->boneAnimNums[i] = 0;
      this->boneTimes[i] = 0;
      this->bonePlaying[i] = true;
   }
}

BoneController::~BoneController() {}

void BoneController::rotateBone(int boneIndex, float angle, Eigen::Vector3f axis) {
   Eigen::Quaternionf rotQuat(Eigen::AngleAxisf(angle, axis));
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

   model->hasBoneTree ?
      computeRecursiveTransforms(model->boneRoot, Eigen::Matrix4f::Identity()) :
      computeFlatTransforms();
}

void BoneController::computeFlatTransforms() {
   for (int boneIndex = 0; boneIndex < model->boneCount; boneIndex++) {
      int animNum = boneAnimNums[boneIndex];
      float tickTime = boneTimes[boneIndex];

      Bone * bone = & model->bones[boneIndex];
      Animation * anim = & model->animations[animNum];
      AnimBone * animBone = & anim->animBones[boneIndex];

      Eigen::Matrix4f animKeysM = computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);
      Eigen::Matrix4f rotationM = makeRotationMatrix(boneRotations[boneIndex]);

      boneTransforms[boneIndex] = animKeysM * bone->invBonePose;
   }
}

void BoneController::computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = boneAnimNums[boneIndex];
   float tickTime = boneTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   Eigen::Matrix4f animKeysM = computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);
   Eigen::Matrix4f rotationM = makeRotationMatrix(boneRotations[boneIndex]);

   Eigen::Matrix4f accumM = parentM * animKeysM * rotationM;
   boneTransforms[boneIndex] = accumM * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeRecursiveTransforms(bone->childIndices[i], accumM);
}


