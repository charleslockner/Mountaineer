#ifndef __MODEL_H__
#define __MODEL_H__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#define MAX_BONES 100

typedef struct {
   float time;
   glm::vec3 position;
   glm::quat rotation;
   glm::vec3 scale;
} Key;

typedef struct {
   Key * keys;
} AnimBone;

typedef struct {
   float duration;
   unsigned keyCount;
   AnimBone * animBones;
} Animation;

typedef struct {
   short parentIndex;
   short childCount;
   short * childIndices;
   glm::mat4 invBonePose;
   glm::mat4 parentOffset;
} Bone;

typedef struct {
   unsigned int vertexCount;
   unsigned int indexCount;
   unsigned int boneCount;
   unsigned int animationCount;

   unsigned int posID, normID, colorID, uvID,
                tanID, bitanID, indID,
                texID, bIndID, bWeightID;

   int hasNormals, hasColors, hasTexCoords,
       hasTansAndBitans, hasBones, hasAnimations;

   short boneRoot;
   Bone * bones;
   Animation * animations;
} Model;

#endif // __MODEL_H__
