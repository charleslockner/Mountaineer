#ifndef __ENTITY_RIGID_H__
#define __ENTITY_RIGID_H__

#include "entity.h"

#define NBODIES 1

struct RigidBody {
   /* Constant quantities */
   float              mass;               /* mass M */
   Eigen::Matrix3f    Ibody;              /* inertia tensor (in body space) */
   Eigen::Matrix3f    Ibodyinv;           /* inverse of the inertia tensor (in body space) */

   /* State variables */
   Eigen::Vector3f    translation;
   Eigen::Quaternionf orientation;
   Eigen::Vector3f    linearMomentum;
   Eigen::Vector3f    angularMomentum;

   /* Computed quantities */
   Eigen::Vector3f    force;              /* total force acting on the body */
   Eigen::Vector3f    torque;             /* total torque acting on the body */
};

class RigidEntity : public StaticEntity {
public:
   RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model);
   RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model);
   RigidEntity(Eigen::Vector3f pos, Model * model);

   void update(float timeDelta);
   float getLinearEnergy(RigidBody * rb);
   float getRotationalEnergy(RigidBody * rb);

   RigidBody bodies[NBODIES];
};




#endif /* __ENTITY_RIGID_H__ */