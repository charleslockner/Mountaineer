
#include "entity.h"

#include <vector>
#include <assert.h>
#include "animation.h"

// ------------------------ IK Bone ------------------------ //
IKLimb::IKLimb() {}
IKLimb::~IKLimb() {}
// --------------------------------------------------------- //
// ======================= IK Entity ======================= //
// --------------------------------------------------------- //
IKEntity::IKEntity(Eigen::Vector3f pos, Model * model)
: BonifiedEntity(pos, model) {
   this->ikLimbs = std::vector<IKLimb *>(0);
   this->ikBones = std::vector<IKBone>(model->boneCount);
   for (int i = 0; i < model->boneCount; i++) {
      this->ikBones[i].angles = std::vector<double>(model->bones[i].joints.size());
      this->ikBones[i].limbs = std::vector<IKLimb *>(0);
   }
   usingIK = false;
}

void IKEntity::addLimb(std::vector<int> boneIndices, Eigen::Vector3f offset, bool isBase) {
   assert(boneIndices.size() > 0);

   IKLimb * limb = new IKLimb();
   limb->isBase = isBase;
   limb->boneIndices = boneIndices;
   limb->offset = offset;
   limb->goal = Eigen::Vector3f(0,0,0);
   limb->jointAngles = constructJointAnglePtrs(boneIndices);
   this->ikBones[boneIndices[0]].limbs.push_back(limb);
   this->ikLimbs.push_back(limb);
}

void IKEntity::setLimbGoal(int limbIndex, Eigen::Vector3f goal) {
   this->ikLimbs[limbIndex]->goal = goal;
}

void IKEntity::update(float tickDelta) {
   if (model->hasBoneTree && model->hasAnimations) {
      BonifiedEntity::replayIfNeeded(tickDelta);
      computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
   }
}

void IKEntity::animateWithIK() {
   usingIK = true;
}

void IKEntity::animateWithKeyframes() {
   usingIK = false;
}

void IKEntity::computeAnimMs(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = this->animNums[boneIndex];
   float tickTime = this->animTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   IKBone * ikBone = & this->ikBones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   // Compute rotation angles for all limbs who's root starts at this bone
   if (usingIK && ikBone->limbs.size())
      solveLimbs(parentM, ikBone->limbs);

   // If this bone has a computed ik rotation, use it, otherwise use the animation rotation
   Eigen::Matrix4f animM = (usingIK) ? (
      (bone->joints.size() > 0) ?
         animM = constructJointMatrix(boneIndex) :
         animM = bone->parentOffset) :
      AN::ComputeKeyframeTransform(animBone, anim->keyCount, tickTime, anim->duration);

   boneMs[boneIndex] = parentM * animM;
   animMs[boneIndex] = boneMs[boneIndex] * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeAnimMs(bone->childIndices[i], boneMs[boneIndex]);
}

std::vector<double *> IKEntity::constructJointAnglePtrs(std::vector<int>& boneIndices) {
   std::vector<double *> jointAngles = std::vector<double *>();
   for (int i = 0; i < boneIndices.size(); i++) {
      IKBone * ikBone = & ikBones[boneIndices[i]];
      for (int j = 0; j < ikBone->angles.size(); j++)
         jointAngles.push_back(& ikBone->angles[j]);
   }
   return jointAngles;
}

Eigen::Matrix4f IKEntity::constructJointMatrix(int boneIndex) {
   Bone * bone = & model->bones[boneIndex];
   IKBone * ikBone = & ikBones[boneIndex];

   Eigen::Matrix4f jointRotationM = Eigen::Matrix4f::Identity();
   for (int i = 0; i < bone->joints.size(); i++) {
      assert(bone->joints.size() == ikBone->angles.size());
      jointRotationM *= Mmath::AngleAxisMatrix<float>(ikBone->angles[i], bone->joints[i].axis);
   }

   return bone->parentOffset * jointRotationM;
}
