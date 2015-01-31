#ifndef __BONE_CONTROLLER_H__
#define __BONE_CONTROLLER_H__

#include "model.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class BoneController {
public:
   BoneController(Model * model, glm::mat4 * boneTransforms, glm::quat * boneRotations);
   ~BoneController();

   void rotateBone(int boneNum, float angle, glm::vec3 axis);

private:
   Model * model;
   glm::mat4 * boneTransforms;
   glm::quat * boneRotations;
};

#endif // __BONE_CONTROLLER_H__