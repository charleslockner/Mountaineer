
struct RigidBody {
   /* Constant quantities */
   float              mass;               /* mass M */
   Eigen::Matrix3f    Ibody,              /* inertia tensor (in body space) */
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

#define NBODIES 1

void explicit_euler_step(RigidBody * bodies[], float dt) {
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
   }
}

float rotationalEnergy(RigidBody * rb) {
   Eigen::Matrix3f Iworldinv = rb->orientation * rb->Ibodyinv * rb->orientation.conjugate();
   Eigen::Vector3f omega = Iworldinv * rb->angularMomentum;
   return 0.5f * omega.transpose() * (rb->orientation * rb->Ibody * orientation.conjugate()) * omega;
}

void RunSimulation() {
   RigidBody Bodies[NBODIES];

   for (double t = 0; t < 10.0; t += 1.0/30.0) {

      explicit_euler_step(Bodies, 1.0/30.0);
      DisplayBodies();
   }
}

