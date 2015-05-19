#include "climber.h"
#include "spline.h"
#include "assert.h"

Climber::Climber(Eigen::Vector3f pos, Model * model)
: IKEntity(pos, model) {}

void Climber::update(float timeDelta) {
   computeLimbGoals(timeDelta);
   IKEntity::update(timeDelta);
}

void Climber::computeLimbGoals(float timeDelta) {
   // if (ikLimbs.size() == 4) {
   //    IKLimb * leftArm = ikLimbs[0];
   //    IKLimb * rightArm = ikLimbs[1];
   //    IKLimb * leftLeg = ikLimbs[2];
   //    IKLimb * rightLeg = ikLimbs[3];

   //    leftArm->goal

   // } else {
   //    printf("We don't have the right number of limbs\n");
   // }
}