#ifndef __BONE_CONTROLLER_H__
#define __BONE_CONTROLLER_H__

#include "model.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class BoneController {
public:
   BoneController(Model * model, glm::mat4 * boneTransforms);
   ~BoneController();

   void rotateBone(int boneNum, float angle, glm::vec3 axis);
   void playAnimation(int boneNum, int animNum);
   void stopAnimation(int boneNum);
   void updateTransforms(float timeDelta);

private:
   Model * model;
   glm::mat4 * boneTransforms;

   glm::quat boneRotations[MAX_BONES];
   int boneAnimNums[MAX_BONES];
   float boneTimes[MAX_BONES];
   bool bonePlaying[MAX_BONES];

   void computeBoneTransform(int boneIndex, glm::mat4 parentM);
};

#endif // __BONE_CONTROLLER_H__