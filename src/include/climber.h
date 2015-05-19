#ifndef __CLIMBER_H__
#define __CLIMBER_H__

#include "entity.h"

class Climber : public IKEntity {
public:
   Climber(Eigen::Vector3f pos, Model * model);
   void update(float timeDelta);

protected:
   void computeLimbGoals(float timeDelta);
};

#endif // __CLIMBER_H__