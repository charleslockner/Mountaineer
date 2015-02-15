#ifndef __ANIMATION__
#define __ANIMATION__

#include "model.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

typedef void (*AtEndCallback)();

class AnimationHandler {
public:
   AnimationHandler(Model * model, glm::mat4 * boneTransforms);
   ~AnimationHandler();

   void repeat(int animNum);
   void playOnce(int animNum);
   void updateTransforms(float timeDelta);

private:
   Model * model;
   glm::mat4 * boneTransforms;
   int curAnimNum;
   float curTime;
   bool repeating;
};

void computeAnimationBoneTransforms(glm::mat4 * out, Model * model, int animNum, float time);

#endif // __ANIMATION__