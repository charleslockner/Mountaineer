#ifndef __BONE_CONTROLLER_H__
#define __BONE_CONTROLLER_H__

#include "matrix_math.h"
#include "model.h"
#include "ik_solver.h"

#include <vector>

typedef struct {
   std::vector<float> angles;
   int animIndex;
   bool animIsPlaying;
   float animTime;
} EntityBone;

typedef struct {
   std::vector<short> baseBoneIndices; // bones that move the root around
   std::vector<short> reachBoneIndices; // bones that move the end effector
   Eigen::Vector3f baseOffset;
   Eigen::Vector3f reachOffset;
   Eigen::Vector3f baseGoal;
   Eigen::Vector3f reachGoal;
} EntityLimb;

class BoneController {
public:
   BoneController(
      Model * model,
      Eigen::Matrix4f * boneTransforms,
      Eigen::Matrix4f * animTransforms,
      std::vector<EntityLimb> limbs
   );
   ~BoneController();

   void rotateBone(int boneNum, float angle, Eigen::Vector3f axis);
   void playAnimation(int boneNum, int animNum, bool recursive);
   void stopAnimation(int boneNum, bool recursive);
   void setLimbGoal(int limbIndex, Eigen::Vector3f goal);
   void setModelM(Eigen::Matrix4f modelM);
   void updateTransforms(float timeDelta);

private:
   Model * model;
   Eigen::Matrix4f * boneTransforms;
   Eigen::Matrix4f * animTransforms;

   Eigen::Matrix4f modelM;
   std::vector<EntityBone> bones;
   std::vector<EntityLimb> limbs;

   void computeFlatTransforms();
   void computeRecursiveTransforms(int boneIndex, Eigen::Matrix4f parentM);
   std::vector<float *> constructAnglePtrs(int limbIndex) ;
   Eigen::Matrix4f constructJointMatrix(int boneIndex);

};

#endif // __BONE_CONTROLLER_H__
