#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "matrix_math.h"
#include "model.h"
#include <vector>

#define LEFT_BASE    Eigen::Vector3f(1,0,0)
#define UP_BASE      Eigen::Vector3f(0,1,0)
#define FORWARD_BASE Eigen::Vector3f(0,0,1)

struct Model;

class Entity {
public:
   Eigen::Vector3f    position;
   Eigen::Quaternionf rotation;

   Entity(Eigen::Vector3f pos, Eigen::Quaternionf rot);
   Entity(Eigen::Vector3f pos);
   virtual ~Entity();

   void moveAlong(Eigen::Vector3f dir, float dist);
   void moveLeft(float dist);
   void moveRight(float dist);
   void moveForward(float dist);
   void moveBackward(float dist);
   void moveUp(float dist);
   void moveDown(float dist);
   void rotateAlong(float angle, Eigen::Vector3f axis);
   void lookAt(Eigen::Vector3f target);

   Eigen::Vector3f getLeft();
   Eigen::Vector3f getUp();
   Eigen::Vector3f getForward();
};

class ModelEntity : public Entity {
public:
   Eigen::Vector3f scale;
   Model * model;

   ModelEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   ModelEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   ModelEntity(Eigen::Vector3f pos, Model * model);

   Eigen::Matrix4f generateModelM();
};

class AnimatedEntity : public ModelEntity {
public:
   Eigen::Matrix4f animMs[MAX_BONES];   // invBindPose included

   AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   AnimatedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   AnimatedEntity(Eigen::Vector3f pos, Model * model);
   ~AnimatedEntity();
   virtual void update(float timeDelta)=0;
   virtual void playAnimation(int animNum)=0;
   virtual void stopAnimation()=0;
};

class MocapEntity : public AnimatedEntity {
public:
   MocapEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   MocapEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   MocapEntity(Eigen::Vector3f pos, Model * model);

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

class IKEntity : public BonifiedEntity {
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


#endif // __ENTITY_H__
