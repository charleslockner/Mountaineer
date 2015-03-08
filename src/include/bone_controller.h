#ifndef __BONE_CONTROLLER_H__
#define __BONE_CONTROLLER_H__

#include "matrix_math.h"
#include "model.h"
#include "ik_solver.h"

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
   IKSolver * solver;

   float boneAngles[MAX_BONES][MAX_BONE_JOINTS];
   int boneAnimNums[MAX_BONES];
   float boneTimes[MAX_BONES];
   bool bonePlaying[MAX_BONES];

   void computeFlatTransforms();
   void computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM);
   Eigen::Matrix4f constructJointMatrix(int boneIndex);
};

#endif // __BONE_CONTROLLER_H__
