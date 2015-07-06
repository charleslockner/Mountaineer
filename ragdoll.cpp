
typedef struct Joint {
   Eigen::Vector3f axis;
   double position;     // angular position
   double velocity;     // angular velocity
   double acceleration; // angular acceleration
} Joint;


void RigidEntity::step() {
   // iterate from 0 to leaf joints
   // iterate back from leaves to 0
}

void RigidEntity::updateJoint(Joint * j, double dt) {


   double angVelNext = j->velocity + j->acceleration * dt;
   double angPosNext = j->position + j->velocity * dt + 0.5 * j->acceleration * dt * dt;
}
