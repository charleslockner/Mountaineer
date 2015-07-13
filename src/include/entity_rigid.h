#ifndef __ENTITY_RIGID_H__
#define __ENTITY_RIGID_H__

#include "entity.h"

#define NBODIES 1

class RigidEntity : public SkinnedEntity {
private:
   // RigidBody/Joint values that are specific to an entity
   class RJoint {
   public:
      float angle;
      Eigen::Vector3f angularVelocity;
      Eigen::Vector3f angularAcceleration;
      RJoint();
   };

   class RBone {
   public:
      Eigen::Vector3f     force;       /* total force acting on the body */
      Eigen::Vector3f     torque;      /* total torque acting on the body */
      std::vector<RJoint> rjoints;     /* the entity-specific joints at this bone */
      RBone();
   };

   std::vector<RBone> rbones;

   void updateJoint(int boneNdx);

public:
   RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   RigidEntity(Eigen::Vector3f pos, Model * model);

   void update(float timeDelta);
   void applyForce(int boneNum, Eigen::Vector3f force);
   void applyTorque(int boneNum, Eigen::Vector3f torque);
};

#endif /* __ENTITY_RIGID_H__ */