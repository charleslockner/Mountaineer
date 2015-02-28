#ifndef __BONE_CONTROLLER_H__
#define __BONE_CONTROLLER_H__

#include "matrix_math.h"
#include "model.h"

class BoneController {
public:
   BoneController(Model * model, Eigen::Matrix4f * boneTransforms);
   ~BoneController();

   void rotateBone(int boneNum, float angle, Eigen::Vector3f axis);
   void playAnimation(int boneNum, int animNum, bool recursive);
   void stopAnimation(int boneNum, bool recursive);
   void updateTransforms(float timeDelta);

private:
   Model * model;
   Eigen::Matrix4f * boneTransforms;

   Eigen::Quaternionf boneRotations[MAX_BONES];
   int boneAnimNums[MAX_BONES];
   float boneTimes[MAX_BONES];
   bool bonePlaying[MAX_BONES];

   void computeFlatTransforms();
   void computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM);
};

#endif // __BONE_CONTROLLER_H__
