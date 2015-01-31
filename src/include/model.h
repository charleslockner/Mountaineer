#ifndef __MODEL_H__
#define __MODEL_H__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#define MAX_BONES 100

typedef struct {
   float time;
   glm::quat value;
} QuatKey;

typedef struct {
   float time;
   glm::vec3 value;
} Vec3Key;

typedef struct {
   unsigned int translateKeyCount;
   Vec3Key * translateKeys;
   unsigned int rotateKeyCount;
   QuatKey * rotateKeys;
   unsigned int scaleKeyCount;
   Vec3Key * scaleKeys;
} AnimBone;

typedef struct {
   float duration;
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

   short boneRoot;
   Bone * bones;
   Animation * animations;
} Model;

#endif // __MODEL_H__
