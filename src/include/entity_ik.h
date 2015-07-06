
#ifndef __ENTITY_IK_H__
#define __ENTITY_IK_H__

#include "entity.h"

class IKLimb {
public:
   std::vector<int> boneIndices;
   Eigen::Vector3f offset;
   bool isBase;

   Eigen::Vector3f goal;
   std::vector<double *> jointAngles;

   IKLimb();
   ~IKLimb();
};

typedef struct {
   std::vector<double> angles;
   std::vector<IKLimb *> limbs; // list of limbs whos root start at this bone
} IKBone;

class IKEntity : public SkinnedEntity {
public:
   IKEntity(Eigen::Vector3f pos, Model * model);
   void addLimb(std::vector<int> boneIndices, Eigen::Vector3f offset, bool isBase);
   void setLimbGoal(int limbIndex, Eigen::Vector3f goal);
   void update(float timeDelta);
   void animateWithIK();
   void animateWithKeyframes();

   std::vector<IKLimb *> ikLimbs;

protected:
   std::vector<IKBone> ikBones;
   bool usingIK;

   std::vector<double *> constructJointAnglePtrs(std::vector<int>& boneIndices);
   Eigen::Matrix4f constructJointMatrix(int boneIndex);
   void computeAnimMs(int boneIndex, Eigen::Matrix4f parentM);

   void solveLimbs(Eigen::Matrix4f baseM, std::vector<IKLimb *> limbs);
};

#endif // __ENTITY_IK_H__