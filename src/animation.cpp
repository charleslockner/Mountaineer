
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "animation.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

static glm::vec3 lerpVec3(glm::vec3 begin, glm::vec3 end, float ratio) {
   return begin + ratio * (end - begin);
}

static int findEarlyVec3KeyIndex(Vec3Key * keys, int keyCount, float tickTime) {
   for (int i = 0; i < keyCount - 1; i++)
      if (tickTime < keys[i + 1].time)
         return i;

   return keyCount - 2;
}

static int findEarlyQuatKeyIndex(QuatKey * keys, int keyCount, float tickTime) {
   for (int i = 0; i < keyCount - 1; i++)
      if (tickTime < keys[i + 1].time)
         return i;

   return keyCount - 2;
}

static glm::vec3 interpolateVec3(Vec3Key * keys, int keyCount, float tickTime) {
   // we need at least two values to interpolate...
   if (keyCount == 1)
      return keys[0].value;

   int earlyNdx = findEarlyVec3KeyIndex(keys, keyCount, tickTime);
   // printf("%f\n", tickTime);
   int lateNdx = earlyNdx + 1;

   float ratio = (tickTime - keys[earlyNdx].time) / (keys[lateNdx].time - keys[earlyNdx].time);
   ratio = fmin(1.0f, fmax(0.0f, tickTime));

   return lerpVec3(keys[earlyNdx].value, keys[lateNdx].value, ratio);
}

static glm::quat interpolateQuat(QuatKey * keys, int keyCount, float tickTime) {
   // we need at least two values to interpolate...
   if (keyCount == 1)
      return keys[0].value;

   int earlyNdx = findEarlyQuatKeyIndex(keys, keyCount, tickTime);
   int lateNdx = earlyNdx + 1;

   float ratio = (tickTime - keys[earlyNdx].time) / (keys[lateNdx].time - keys[earlyNdx].time);
   ratio = fmin(1.0f, fmax(0.0f, tickTime));

   return glm::slerp(keys[earlyNdx].value, keys[lateNdx].value, ratio);
}

static glm::mat4 computeAnimTransform(AnimBone * animBone, float time) {
   glm::vec3 scaleVec = interpolateVec3(animBone->scaleKeys, animBone->scaleKeyCount, time);
   glm::quat rotateQuat = interpolateQuat(animBone->rotateKeys, animBone->rotateKeyCount, time);
   glm::vec3 transVec = interpolateVec3(animBone->translateKeys, animBone->translateKeyCount, time);

   // printf("transVec %f %f %f\n", transVec.x, transVec.y, transVec.z);

   glm::mat4 scaleM = glm::scale(glm::mat4(1.0), scaleVec);
   glm::mat4 rotateM = glm::toMat4(rotateQuat);
   glm::mat4 transM = glm::translate(glm::mat4(1.0), transVec);

   // return translateM;
   return transM * rotateM * scaleM;
}

static void computeBoneTransform(glm::mat4 * transforms, Bone * tree, int boneIndex,
                                 AnimBone * animBones, glm::mat4 parentM, float time) {

   Bone * bone = & tree[boneIndex];
   AnimBone * animBone = & animBones[boneIndex];
   glm::mat4 inverseBonePose = bone->offset;

   glm::mat4 animM = parentM * computeAnimTransform(animBone, time);
   transforms[boneIndex] = animM * inverseBonePose;

   // transforms[boneIndex] = glm::mat4(1);

   for (int i = 0; i < bone->childCount; i++)
      computeBoneTransform(transforms, tree, bone->childIndices[i], animBones, animM, time);
}

void computeBoneTransforms(glm::mat4 * transforms, Model * model, int animNum, float time) {
   Animation * anim = & model->animations[animNum];

   if (animNum < 0 || animNum > model->animationCount) {
      printf("Trying to access animNum %d when the model only has %d animations\n", animNum, model->animationCount);
      exit(1);
   }
   // printf("%f\n", time);
   if (time > anim->duration) {
      printf("Animation (%d) time %f is greater than its duration %f\n", animNum, time, anim->duration);
      exit(1);
   }

   // recursively fill in each bone transform
   computeBoneTransform(transforms, model->bones, model->boneRoot, anim->animBones, glm::mat4(1.0f), time);
}

