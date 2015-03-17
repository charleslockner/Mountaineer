
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
   this->bones = std::vector<EntityBone>(model->boneCount);

   for (int i = 0; i < model->boneCount; i++) {
      this->bones[i].angles = std::vector<float>(model->bones[i].joints.size());
      // for (int j = 0; j < model->bones[i].joints.size(); j++)
      //    this->bones[i].angles.push_back(0);
      this->bones[i].animIndex = 0;
      this->bones[i].animTime = 0;
      this->bones[i].animIsPlaying = false;
   }
   this->limbs = std::vector<EntityLimb>(model->limbSolvers.size());
   this->modelM = Eigen::Matrix4f::Identity();
}

BoneController::~BoneController() {}

void BoneController::playAnimation(int boneNum, int animNum, bool recursive) {
   assert(animNum < model->animationCount);
   bones[boneNum].animIndex = animNum;
   // bones[boneNum].animTime = 0;
   bones[boneNum].animIsPlaying = true;

   if (recursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         playAnimation(model->bones[boneNum].childIndices[i], animNum, true);
}

void BoneController::stopAnimation(int boneNum, bool recursive) {
   bones[boneNum].animIsPlaying = false;

   if (recursive)
      for (int i = 0; i < model->bones[boneNum].childCount; i++)
         stopAnimation(model->bones[boneNum].childIndices[i], true);
}

void BoneController::updateTransforms(float tickDelta) {
   for (int i = 0; i < model->boneCount; i++) {
      if (!model->hasBoneTree || bones[i].animIsPlaying) {
         float duration = model->animations[bones[i].animIndex].duration;

         bones[i].animTime += tickDelta;
         if (bones[i].animTime > duration)
            bones[i].animTime -= duration;
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
      int animNum = bones[boneIndex].animIndex;
      float tickTime = bones[boneIndex].animTime;

      Bone * bone = & model->bones[boneIndex];
      Animation * anim = & model->animations[animNum];
      AnimBone * animBone = & anim->animBones[boneIndex];

      // We cannot manually rotate bones if there is no bone hierarchy.
      Eigen::Matrix4f animKeysM = computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);
      boneTransforms[boneIndex] = animKeysM * bone->invBonePose;
   }
}

void BoneController::setLimbGoal(int limbIndex, Eigen::Vector3f goal) {
   this->limbs[limbIndex].goal = goal;
}

void BoneController::setModelM(Eigen::Matrix4f modelM) {
   this->modelM = modelM;
}

std::vector<float *> BoneController::constructAnglePtrs(int limbIndex) {
   std::vector<float *> angles = std::vector<float *>();
   std::vector<short> boneIndices = model->limbSolvers[limbIndex]->boneIndices;
   for (int i = 0; i < boneIndices.size(); i++) {
      EntityBone * bone = & bones[boneIndices[i]];
      for (int j = 0; j < bone->angles.size(); j++)
         angles.push_back(& bone->angles[j]);
   }
   return angles;
}

Eigen::Matrix4f BoneController::constructJointMatrix(int boneIndex) {
   Bone * bone = & model->bones[boneIndex];
   EntityBone * entBone = & bones[boneIndex];

   Eigen::Matrix4f jointRotationM = Eigen::Matrix4f::Identity();
   for (int i = 0; i < bone->joints.size(); i++) {
      assert(bone->joints.size() == entBone->angles.size());
      jointRotationM *= Mmath::angleAxisMatrix<float>(entBone->angles[i], bone->joints[i].axis);
   }

   return bone->parentOffset * jointRotationM;
}

void BoneController::computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = bones[boneIndex].animIndex;
   float tickTime = bones[boneIndex].animTime;

   Bone * bone = & model->bones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   // if this bone is the root of a limb, compute the limb's ik rotation angles.
   if (bone->limbIndex >= 0) {
      IKSolver * limbSolver = model->limbSolvers[bone->limbIndex];
      Eigen::Vector3f goal = this->limbs[bone->limbIndex].goal;
      std::vector<float *> angles = constructAnglePtrs(bone->limbIndex);
      Eigen::Matrix4f baseM = modelM * parentM;
      limbSolver->solveBoneRotations(baseM, goal, angles);
      angles.clear();
   }

   // if this bone has a computed ik rotation, use it, otherwise use the animation
   Eigen::Matrix4f accumM = bone->joints.size() > 0 ?
      parentM * constructJointMatrix(boneIndex) :
      parentM * computeAnimTransform(animBone, anim->keyCount, tickTime, anim->duration);

   boneTransforms[boneIndex] = accumM * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeRecursiveTransforms(bone->childIndices[i], accumM);
}


