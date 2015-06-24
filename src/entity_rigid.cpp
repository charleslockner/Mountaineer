#include "entity_rigid.h"

RigidEntity::RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Eigen::Vector3f scl, Model * model)
: ModelEntity(pos, rot, scl, model) {}
RigidEntity::RigidEntity(Eigen::Vector3f pos, Eigen::Quaternionf rot, Model * model)
: ModelEntity(pos, rot, model) {}
RigidEntity::RigidEntity(Eigen::Vector3f pos, Model * model)
: ModelEntity(pos, model) {
   for (int i = 0; i < NBODIES; i++) {
      RigidBody * rb = & bodies[i];

      rb->mass = 1;
      rb->Ibody = Eigen::Matrix3f::Identity();
      rb->Ibodyinv = rb->Ibody.inverse();

      rb->translation = Eigen::Vector3f(0,0,0);
      rb->orientation = Eigen::Quaternionf::Identity();

      rb->linearMomentum = Eigen::Vector3f(0,0,0);
      rb->angularMomentum = Eigen::Vector3f(0,0,0);

      rb->force = Eigen::Vector3f(0,0,0);
      rb->torque = Eigen::Vector3f(0,0,0);
   }
}

static float timeTot = 0;

void RigidEntity::update(float dt) {
   for (int i = 0; i < NBODIES; i++) {
      RigidBody * rb = & bodies[i];

      // Update the linear and angular momentums
      rb->linearMomentum  += dt * rb->force;
      rb->angularMomentum += dt * rb->torque;

      // Update the translation
      Eigen::Vector3f velocity = rb->linearMomentum / rb->mass;
      rb->translation += dt * velocity;

      // Update the orientation
      Eigen::Matrix3f Iworldinv = rb->orientation * rb->Ibodyinv * rb->orientation.conjugate();
      Eigen::Vector3f omega = Iworldinv * rb->angularMomentum;
      Eigen::Quaternionf omegaRotation = Eigen::Quaternionf(0.0f, omega(0), omega(1), omega(2)) * rb->orientation;
      rb->orientation.w() += 0.5f * dt * omegaRotation.w();
      rb->orientation.x() += 0.5f * dt * omegaRotation.x();
      rb->orientation.y() += 0.5f * dt * omegaRotation.y();
      rb->orientation.z() += 0.5f * dt * omegaRotation.z();
      rb->orientation.normalize();


      timeTot += dt;
      if (timeTot > 1) {
         timeTot -= 1;

         printf("velocity:  %f %f %f\n", velocity(0), velocity(1), velocity(2));
         printf("translation:  %f %f %f\n", rb->translation(0), rb->translation(1), rb->translation(2));

         printf("linearMomentum:  %f %f %f\n", rb->linearMomentum(0), rb->linearMomentum(1), rb->linearMomentum(2));
         printf("angularMomentum: %f %f %f\n", rb->angularMomentum(0), rb->angularMomentum(1), rb->angularMomentum(2));
         printf("omega: %f %f %f\n", omega(0), omega(1), omega(2));
         printf("omegaRotation: %f %f %f %f\n", omegaRotation.w(), omegaRotation.x(), omegaRotation.y(), omegaRotation.z());
         printf("orientation: %f %f %f %f\n", rb->orientation.w(), rb->orientation.x(), rb->orientation.y(), rb->orientation.z());
         printf("dt: %f, Rot Energy: %f\n\n", dt, getRotationalEnergy(rb));
      }

      position = rb->translation;
      rotation = rb->orientation;
   }
}

float RigidEntity::getRotationalEnergy(RigidBody * rb) {
   Eigen::Matrix3f Iworldinv = rb->orientation * rb->Ibodyinv * rb->orientation.conjugate();
   Eigen::Vector3f omega = Iworldinv * rb->angularMomentum;
   return 0.5f * omega.transpose() * (rb->orientation * rb->Ibody * rb->orientation.conjugate()) * omega;
}