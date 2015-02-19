#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"
#include "shader.h"
#include "bone_controller.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class EntityShader;

class Entity {
public:
   glm::vec3 position;
   // glm::quat rotation;
   float rotation;
   glm::vec3 scale;

   Model * model;
   glm::mat4 boneTransforms[MAX_BONES];

   Entity(glm::vec3 pos, Model * model);
   ~Entity();

   void update(float time);
   glm::mat4 generateModelM();

private:
   BoneController * boneController;
   glm::quat boneRotations[MAX_BONES];
};

#endif // __ENTITY_H__