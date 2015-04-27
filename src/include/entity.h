#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"
#include "shader.h"

#include <vector>

// Forward declare this shiz
class EntityShader;
struct Model;

class Entity {
public:
   Eigen::Vector3f position;
   Eigen::Quaternionf rotation;
   Eigen::Vector3f scale;
   Model * model;

   Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   Entity(Eigen::Vector3f pos, Model * model);
   ~Entity();

   virtual void update(float timeDelta)=0;
   Eigen::Matrix4f generateModelM();
};


class AnimatedEntity : public Entity {
public:
   Eigen::Matrix4f animMs[MAX_BONES];   // invBindPose included

   // AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   // AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   AnimatedEntity(Eigen::Vector3f pos, Model * model);

   virtual void playAnimation(int animNum)=0;
   virtual void stopAnimation()=0;
};

class BonelessEntity : public AnimatedEntity {
public:
   // BonelessEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   // BonelessEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   BonelessEntity(Eigen::Vector3f pos, Model * model);

   void playAnimation(int animNum);
   void stopAnimation();
   void update(float timeDelta);

private:
   bool animIsPlaying;
   int animNum;
   float animTime;
};


class BonifiedEntity : public AnimatedEntity {
public:
   // has all space transforms except for invBindPose
   std::vector<Eigen::Matrix4f> boneMs;

   BonifiedEntity(Eigen::Vector3f pos, Model * model);
   void playAnimation(int animNum);
   void playAnimation(int animNum, int boneNum, bool recursive);
   void stopAnimation();
   void stopAnimation(int boneNum, bool recursive);
   void update(float timeDelta);

protected:
   std::vector<int> animNums;
   std::vector<bool> bonesPlaying;
   std::vector<float> animTimes;

   void replayIfNeeded(float timeDelta);
   void computeAnimMs(int boneIndex, Eigen::Matrix4f parentM);
};

class IKLimb {
public:
   std::vector<int> boneIndices;
   Eigen::Vector3f offset;
   Eigen::Vector3f goal;
   std::vector<double *> jointAngles;
   bool isBase;

   IKLimb();
   ~IKLimb();
};

typedef struct {
   std::vector<double> angles;
   std::vector<IKLimb *> limbs; // list of limbs whos root start at this bone
} IKBone;

class IKEntity : public BonifiedEntity {
public:
   IKEntity(Eigen::Vector3f pos, Model * model);
   void addLimb(std::vector<int> boneIndices, Eigen::Vector3f offset, bool isBase);
   void setLimbGoal(int limbIndex, Eigen::Vector3f goal);
   void update(float timeDelta);

protected:
   std::vector<IKLimb *> ikLimbs;
   std::vector<IKBone> ikBones;

   std::vector<double *> constructJointAnglePtrs(std::vector<int>& boneIndices);
   Eigen::Matrix4f constructJointMatrix(int boneIndex);
   void computeAnimMs(int boneIndex, Eigen::Matrix4f parentM);

   void solveLimbs(Eigen::Matrix4f baseM, std::vector<IKLimb *> limbs);
};


#endif // __ENTITY_H__
