#ifndef __ANIMATION__
#define __ANIMATION__

#include "model.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

void computeBoneTransforms(glm::mat4 * transforms, Model * model, int animNum, float time);

#endif // __ANIMATION__