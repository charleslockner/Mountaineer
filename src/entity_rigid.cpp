#include "entity_rigid.h"

RigidEntity::RJoint::RJoint()
: angle(0), angularVelocity(Eigen::Vector3f(0,0,0)), angularAcceleration(Eigen::Vector3f(0,0,0)) {}
RigidEntity::RBone::RBone()
: force(Eigen::Vector3f(0,0,0)), torque(Eigen::Vector3f(0,0,0)) {}

RigidEntity::RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: SkinnedEntity(pos, rot, scl, model) {}
RigidEntity::RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: SkinnedEntity(pos, rot, model) {}
RigidEntity::RigidEntity(Eigen::Vector3f pos, Model * model)
: SkinnedEntity(pos, model) {
   // Initialize the rigid bones and joints
   for (int i = 0; i < model->bones.size(); i++) {
      RBone rbone;
      rbone.rjoints = std::vector<RJoint>(model->bones[i].joints.size());
      rbones.push_back(rbone);
   }
}

typedef struct FT {
   Eigen::Vector3f force;
   Eigen::Vector3f torque;
} FT;

void RigidEntity::update(float timeDelta) {
   updateJoint(model->boneRoot);
}

   // Use these
   // mBone->mass;
   // mBone->inertiaTensor;
   // mBone->invInertiaTensor;
   // mBone->com;

void RigidEntity::updateJoint(int boneNdx) {
   Bone * mBone = & model->bones[boneNdx];
   RBone * rBone = & rbones[boneNdx];

   for (int i = 0; i < mBone->childIndices.size(); i++) {
      int childIndex = mBone->childIndices[i];
      Bone *  mChildBone = & model->bones[childIndex];
      RBone * rChildBone = & rbones[childIndex];
      Eigen::Matrix3f rotChildM = Mmath::AngleAxisMatrix3(rChildBone->rjoints[0].angle, mBone->joints[0].axis);

      // Calculate child's linear & angular velocities and accelerations
      // rChildBone->angularVelocity     = rotChildM * rBone->angularVelocity + childAngleDot * zhat;
      // rChildBone->angularAcceleration = rotChildM * rBone->angularAcceleration +
      //                                   (rotChildM * rBone->angularVelocity).cross(childAngleDot * zhat) +
      //                                   childAngleDotDot * zhat;
      // rChildBone->linearVelocity      =
      // rChildBone->linearAcceleration  =

      // Calculate the child forces and torques required for this body's force/torques
      updateJoint(childIndex);

      // Calculate this body's force/torques
      rBone->force  += rotChildM * rChildBone->force;
      // rBone->torque +=
   }

   // double angVelNext = j->velocity + j->acceleration * dt;
   // double angPosNext = j->position + j->velocity * dt + 0.5 * j->acceleration * dt * dt;
}

void RigidEntity::applyForce(int boneNum, Eigen::Vector3f f) {
   rbones[boneNum].force += f;
}

void RigidEntity::applyTorque(int boneNum, Eigen::Vector3f t) {
   rbones[boneNum].torque += t;
}
