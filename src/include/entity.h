#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "matrix_math.h"
#include "model.h"
#include <vector>

#define LEFT_BASE       (Eigen::Vector3f(1,0,0))
#define RIGHT_BASE      (Eigen::Vector3f(-1,0,0))
#define UP_BASE         (Eigen::Vector3f(0,1,0))
#define DOWN_BASE       (Eigen::Vector3f(0,-1,0))
#define FORWARD_BASE    (Eigen::Vector3f(0,0,1))
#define BACKWARD_BASE   (Eigen::Vector3f(0,0,-1))

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
   Eigen::Vector3f getRight();
   Eigen::Vector3f getDown();
   Eigen::Vector3f getUp();
   Eigen::Vector3f getForward();
   Eigen::Vector3f getBackward();
};

class StaticEntity : public Entity {
public:
   Eigen::Vector3f scale;
   Model * model;

   /* rigid body quantities */
   Eigen::Vector3f    linearMomentum;
   Eigen::Vector3f    angularMomentum;
   Eigen::Vector3f    force;              /* total force acting on the body */
   Eigen::Vector3f    torque;             /* total torque acting on the body */

   StaticEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   StaticEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   StaticEntity(Eigen::Vector3f pos, Model * model);

   void applyForce(Eigen::Vector3f force);
   void applyTorque(Eigen::Vector3f torque);
   void physicsStep(float timeDelta);

   float getLinearEnergy();
   float getRotationalEnergy();
   Eigen::Matrix4f generateModelM();

protected:
   void initializePhysics();
};

class AnimatedEntity : public StaticEntity {
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

class SkinnedEntity : public AnimatedEntity {
public:
   // has all space transforms except for invBindPose
   std::vector<Eigen::Matrix4f> boneMs;

   SkinnedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   SkinnedEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   SkinnedEntity(Eigen::Vector3f pos, Model * model);

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

private:
   void initialize();
};


#endif // __ENTITY_H__
