
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "animation.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

static glm::vec3 lerpVec3(glm::vec3 begin, glm::vec3 end, float ratio) {
   return begin + ratio * (end - begin);
}

// should improve this to constant time somehow (instead of log(keyCount))
static int findEarlyVec3KeyIndex(Vec3Key * keys, int keyCount, float tickTime) {
   if (keyCount <= 2)
      return 0;

   int halfCount = keyCount / 2;

   if (tickTime < keys[halfCount].time)
      return findEarlyVec3KeyIndex(keys, halfCount + 1, tickTime);
   else
      return halfCount + findEarlyVec3KeyIndex(keys + halfCount, keyCount - halfCount, tickTime);
}

// should improve this to constant time somehow (instead of log(keyCount))
static int findEarlyQuatKeyIndex(QuatKey * keys, int keyCount, float tickTime) {
   if (keyCount <= 2)
      return 0;

   int halfCount = keyCount / 2;

   if (tickTime < keys[halfCount].time)
      return findEarlyQuatKeyIndex(keys, halfCount + 1, tickTime);
   else
      return halfCount + findEarlyQuatKeyIndex(keys + halfCount, keyCount - halfCount, tickTime);
}

static glm::vec3 interpolateVec3(Vec3Key * keys, int keyCount, float tickTime) {
   // we need at least two values to interpolate...
   if (keyCount == 1)
      return keys[0].value;

   int earlyNdx = findEarlyVec3KeyIndex(keys, keyCount, tickTime);
   int lateNdx = earlyNdx + 1;
   float ratio = (tickTime - keys[earlyNdx].time) / (keys[lateNdx].time - keys[earlyNdx].time);

   return lerpVec3(keys[earlyNdx].value, keys[lateNdx].value, ratio);
}

static glm::quat interpolateQuat(QuatKey * keys, int keyCount, float tickTime) {
   // we need at least two values to interpolate...
   if (keyCount == 1)
      return keys[0].value;

   int earlyNdx = findEarlyQuatKeyIndex(keys, keyCount, tickTime);
   int lateNdx = earlyNdx + 1;
   float ratio = (tickTime - keys[earlyNdx].time) / (keys[lateNdx].time - keys[earlyNdx].time);

   return glm::slerp(keys[earlyNdx].value, keys[lateNdx].value, ratio);
}

static glm::mat4 computeAnimTransform(AnimBone * animBone, float time) {
   glm::vec3 scaleVec = interpolateVec3(animBone->scaleKeys, animBone->scaleKeyCount, time);
   glm::quat rotateQuat = interpolateQuat(animBone->rotateKeys, animBone->rotateKeyCount, time);
   glm::vec3 transVec = interpolateVec3(animBone->translateKeys, animBone->translateKeyCount, time);

   glm::mat4 scaleM = glm::scale(glm::mat4(1.0), scaleVec);
   glm::mat4 rotateM = glm::toMat4(rotateQuat);
   glm::mat4 transM = glm::translate(glm::mat4(1.0), transVec);

   // return translateM;
   return transM * rotateM * scaleM;
}

static void computeBoneTransform(glm::mat4 * transforms, Bone * tree, int boneIndex,
                                 AnimBone * animBones, glm::mat4 parentM, float time) {

   Bone * bone = & tree[boneIndex];

   glm::mat4 animKeysM = computeAnimTransform(& animBones[boneIndex], time);
   glm::mat4 animPoseM = bone->parentOffset;
   glm::mat4 bonePoseM = bone->invBonePose;

   // glm::mat4 animM = parentM * animPoseM * animKeysM;
   // glm::mat4 animM = parentM * animPoseM;
   glm::mat4 animM = parentM * animKeysM;
   transforms[boneIndex] = animM * bonePoseM;

   for (int i = 0; i < bone->childCount; i++)
      computeBoneTransform(transforms, tree, bone->childIndices[i], animBones, animM, time);
}

AnimationHandler::AnimationHandler(Model * model, glm::mat4 * boneTransforms) {
   this->model = model;
   this->boneTransforms = boneTransforms;
   this->curAnimNum = 0;
   this->curTime = 0;
   this->repeating = true;
   assert(model->animationCount > 0);
}

AnimationHandler::~AnimationHandler() {}

void AnimationHandler::repeat(int animNum) {
   curAnimNum = animNum;
   curTime = 0;
   repeating = true;
   assert(animNum < model->animationCount);
}

void AnimationHandler::playOnce(int animNum) {
   curAnimNum = animNum;
   curTime = 0;
   repeating = false;
   assert(animNum < model->animationCount);
}

void AnimationHandler::updateTransforms(float timeDelta) {
   Animation * anim = & model->animations[curAnimNum];
   float duration = model->animations[curAnimNum].duration;

   curTime += timeDelta;
   if (curTime > duration)
      curTime = repeating ? curTime - duration : duration;

   // recursively fill in bone transforms
   computeBoneTransform(boneTransforms, model->bones, model->boneRoot, anim->animBones, glm::mat4(1.0f), curTime);
}

