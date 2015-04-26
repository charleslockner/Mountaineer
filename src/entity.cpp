#include "entity.h"
#include "matrix_math.h"
#include "animation.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

// -------------------------------------------------------- //
// ========================= Entity ======================= //
// -------------------------------------------------------- //
Entity::Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: position(pos), rotation(rot), scale(scl), model(model) {}
Entity::Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: Entity(pos, rot, Eigen::Vector3f(1,1,1), model) {}
Entity::Entity(Eigen::Vector3f pos, Model * model)
: Entity(pos, Eigen::Quaternionf(1,0,0,0), Eigen::Vector3f(1,1,1), model) {}
Entity::~Entity() {}

Eigen::Matrix4f Entity::generateModelM() {
   return Mmath::TransformationMatrix(position, rotation, scale);
}

// --------------------------------------------------------- //
// ==================== Animated Entity ==================== //
// --------------------------------------------------------- //
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: Entity(pos, rot, scl, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: Entity(pos, rot, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}
AnimatedEntity::AnimatedEntity(Eigen::Vector3f pos, Model * model)
: Entity(pos, model) {
   for (int i = 0; i < MAX_BONES; i++)
      this->animMs[i] = Eigen::Matrix4f::Identity();
}

// --------------------------------------------------------- //
// ==================== Boneless Entity ==================== //
// --------------------------------------------------------- //
BonelessEntity::BonelessEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: AnimatedEntity(pos, rot, scl, model),
  animNum(0), animTime(0), animIsPlaying(false) {}
BonelessEntity::BonelessEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: AnimatedEntity(pos, rot, model),
  animNum(0), animTime(0), animIsPlaying(false) {}
BonelessEntity::BonelessEntity(Eigen::Vector3f pos, Model * model)
: AnimatedEntity(pos, model),
  animNum(0), animTime(0), animIsPlaying(false) {}

void BonelessEntity::playAnimation(int animNum) {
   animIsPlaying = true;
   this->animNum = animNum;
}

void BonelessEntity::stopAnimation() {
   animIsPlaying = false;
}

void BonelessEntity::update(float tickDelta) {
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
// ==================== Boneified Entity =================== //
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
   // Start replaying animation if finished
   for (int i = 0; i < model->boneCount; i++) {
      if (!model->hasBoneTree || bonesPlaying[i]) {
         float duration = model->animations[animNums[i]].duration;

         animTimes[i] += tickDelta;
         if (animTimes[i] > duration)
            animTimes[i] -= duration;
      }
   }

   // Recursively fill in the animMs
   computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
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

// --------------------------------------------------------- //
// ======================= IK Entity ======================= //
// --------------------------------------------------------- //
IKEntity::IKEntity(Eigen::Vector3f pos, Model * model)
: BonifiedEntity(pos, model) {
   this->ikAngles = std::vector<std::vector<float> >(model->boneCount);
   for (int i = 0; i < model->boneCount; i++)
      this->ikAngles[i] = std::vector<float>(model->bones[i].joints.size());
}

// void BonifiedEntity::update(float tickDelta) {
//    // Start replaying animation if finished
//    for (int i = 0; i < model->boneCount; i++) {
//       if (!model->hasBoneTree || bonesPlaying[i]) {
//          float duration = model->animations[animNums[i]].duration;

//          animTimes[i] += tickDelta;
//          if (animTimes[i] > duration)
//             animTimes[i] -= duration;
//       }
//    }

//    // Recursively fill in the animMs
//    computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
// }

// // std::vector<float *> BoneController::constructAnglePtrs(int limbIndex) {
// //    std::vector<float *> angles = std::vector<float *>();
// //    std::vector<short> boneIndices = limbs[limbIndex].reachBoneIndices;
// //    for (int i = 0; i < boneIndices.size(); i++) {
// //       EntityBone * bone = & bones[boneIndices[i]];
// //       for (int j = 0; j < bone->angles.size(); j++)
// //          angles.push_back(& bone->angles[j]);
// //    }
// //    return angles;
// // }

// // Eigen::Matrix4f BoneController::constructJointMatrix(int boneIndex) {
// //    Bone * bone = & model->bones[boneIndex];
// //    EntityBone * entBone = & bones[boneIndex];

// //    Eigen::Matrix4f jointRotationM = Eigen::Matrix4f::Identity();
// //    for (int i = 0; i < bone->joints.size(); i++) {
// //       assert(bone->joints.size() == entBone->angles.size());
// //       jointRotationM *= Mmath::AngleAxisMatrix<float>(entBone->angles[i], bone->joints[i].axis);
// //    }

// //    return bone->parentOffset * jointRotationM;
// // }


// void BonifiedEntity::computeAnimMs(int boneIndex, Eigen::Matrix4f parentM) {
//    int animNum = bones[boneIndex].animIndex;
//    float tickTime = bones[boneIndex].animTime;

//    Bone * bone = & model->bones[boneIndex];
//    Animation * anim = & model->animations[animNum];
//    AnimBone * animBone = & anim->animBones[boneIndex];

//    // if this bone is the root of a limb, compute the limb's ik rotation angles.
//    // if (bone->limbIndex >= 0) {
//    //    EntityLimb * limb = & this->limbs[bone->limbIndex];
//    //    std::vector<short> boneIndices = limb->reachBoneIndices;
//    //    Eigen::Vector3f goal = limb->reachGoal;
//    //    std::vector<float *> angles = constructAnglePtrs(bone->limbIndex);
//    //    Eigen::Matrix4f baseM = modelM * parentM;
//    //    IK::SolveSegment(model, baseM, goal, angles, boneIndices);
//    // }

//    // if this bone has a computed ik rotation, use it, otherwise use the animation
//    // Eigen::Matrix4f accumM = bone->joints.size() > 0 ?
//    //    parentM * constructJointMatrix(boneIndex) :
//    //    parentM * AN::ComputeKeyframeTransform(animBone, anim->keyCount, tickTime, anim->duration);

//    Eigen::Matrix4f accumM = parentM * AN::ComputeKeyframeTransform(animBone, anim->keyCount, tickTime, anim->duration);
//    boneMs[boneIndex] = accumM;
//    animMs[boneIndex] = accumM * bone->invBonePose;

//    for (int i = 0; i < bone->childCount; i++)
//       computeRecursiveTransforms(bone->childIndices[i], accumM);
// }

// void BonifiedEntity::setLimbGoal(int limbIndex, Eigen::Vector3f goal) {
//    // this->limbs[limbIndex].reachGoal = goal;
// }





