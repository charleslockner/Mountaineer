#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"
#include "shader.h"
#include "bone_controller.h"

#include "matrix_math.h"

class EntityShader;

class Entity {
public:
   Eigen::Vector3f position;
   Eigen::Quaternionf rotation;
   Eigen::Vector3f scale;

   Model * model;
   Eigen::Matrix4f boneTransforms[MAX_BONES];   // has all space transforms except for invBindPose
   Eigen::Matrix4f animTransforms[MAX_BONES];   // invBindPose included

   Entity(Eigen::Vector3f pos, Model * model);
   ~Entity();

   void update(float time);
   Eigen::Matrix4f generateModelM();

   BoneController * boneController;
};

#endif // __ENTITY_H__
