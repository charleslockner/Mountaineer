
#include "bone_controller.h"
#include "matrix_math.h"

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
   std::vector<Key>& keys = animBone->keys;
   assert(keyCount >= 1);

   int earlyNdx = findEarlyKeyIndex(keyCount, tickTime, duration);
   int lateNdx = earlyNdx + 1;

   Key interpKey = (keyCount == 1) ? keys[earlyNdx] :
      interpolateKeys(keys[earlyNdx], keys[lateNdx], tickTime);

   return Mmath::transformationMatrix(interpKey.position, interpKey.rotation, interpKey.scale);
}

BoneController::BoneController(Model * model, Eigen::Matrix4f * boneTransforms) {
   this->model = model;
   this->boneTransforms = boneTransforms;
   this->solver = new IKSolver(model);

   for (int i = 0; i < model->boneCount; i++) {
      for (int j = 0; j < MAX_BONE_JOINTS; j++)
         this->boneAngles[i][j] = 0;
      this->boneAnimNums[i] = 0;
      this->boneTimes[i] = 0;
      this->bonePlaying[i] = false;
   }
}

BoneController::~BoneController() {}

void BoneController::playAnimation(int boneNum, int animNum, bool recursive) {
   assert(animNum < model->animationCount);
   boneAnimNums[boneNum] = animNum;
   boneTimes[boneNum] = 0;
   bonePlaying[boneNum] = true;

   if (recursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         playAnimation(model->bones[boneNum].childIndices[i], animNum, true);
}

void BoneController::stopAnimation(int boneNum, bool recursive) {
   bonePlaying[boneNum] = false;

   if (recursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         stopAnimation(model->bones[boneNum].childIndices[i], true);
}

void BoneController::updateTransforms(float tickDelta) {
   for (int i = 0; i < model->boneCount; i++) {
      if (!model->hasBoneTree || bonePlaying[i]) {
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

// void BoneController::rotateBone(int boneIndex, float angle, Eigen::Vector3f axis) {
//    Eigen::Quaternionf rotQuat(Eigen::AngleAxisf(angle, axis));
//    boneAngles[boneIndex] = boneAngles[boneIndex] * rotQuat;
// }

void BoneController::computeFlatTransforms() {
   for (int boneIndex = 0; boneIndex < model->boneCount; boneIndex++) {
      int animNum = boneAnimNums[boneIndex];
      float tickTime = boneTimes[boneIndex];

      Bone * bone = & model->bones[boneIndex];
      Animation * anim = & model->animations[animNum];
      AnimBone * animBone = & anim->animBones[boneIndex];

      // We cannot manually rotate bones if there is no bone hierarchy.
      Eigen::Matrix4f animKeysM = computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);
      boneTransforms[boneIndex] = animKeysM * bone->invBonePose;
   }
}

Eigen::Matrix4f BoneController::constructJointMatrix(int boneIndex) {
   Bone * bone = & model->bones[boneIndex];
   assert(bone->jointCount <= MAX_BONE_JOINTS);

   Eigen::Matrix4f jointRotationM = Eigen::Matrix4f::Identity();
   for (int i = 0; i < bone->jointCount; i++)
      jointRotationM *= Mmath::angleAxisMatrix(boneAngles[boneIndex][i], bone->joints[i].axis);

   return bone->parentOffset * jointRotationM;
}

void BoneController::computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = boneAnimNums[boneIndex];
   float tickTime = boneTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];
   IKLimb * limb = bone->limb;

   // if this bone is the root joint for a limb, compute its ik rotation angles.
   if (limb)
      solver->solveBoneRotations(limb, & boneAngles[boneIndex][0]);

   // if this bone is part of a limb, use the ik rotation, otherwise use the animation
   Eigen::Matrix4f accumM = bone->jointCount > 0 ?
      parentM * constructJointMatrix(boneIndex) :
      parentM * computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);

   boneTransforms[boneIndex] = accumM * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeRecursiveTransforms(bone->childIndices[i], accumM);
}


