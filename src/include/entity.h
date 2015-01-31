#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"
#include "animation.h"
#include "shader.h"
#include "bone_controller.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class EntityShader;

class Entity {
public:
   glm::vec3 position;
   float rotation;
   glm::vec3 scale;

   Model * model;
   AnimationHandler * animHandler;
   BoneController * boneController;

   glm::mat4 boneTransforms[MAX_BONES];
   glm::quat boneRotations[MAX_BONES];

   Entity(glm::vec3 pos, Model * model);
   ~Entity();

   void draw(EntityShader * shader, float time);
};

#endif // __ENTITY_H__