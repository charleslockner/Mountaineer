#ifndef __MODEL_H__
#define __MODEL_H__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include "stdio.h"

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
   unsigned int fps;
   unsigned int keyCount;
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

class Model {
public:
   Model();
   ~Model();

   void loadCIAB(const char * path);
   void loadTexture(const char * path);
   void loadOBJ(const char * path);
   void loadSkinningPIN(const char * path);
   void loadAnimationPIN(const char * path);

   void printBoneTree();
   void printAnimations();

   unsigned int vertexCount;
   unsigned int indexCount;
   unsigned int boneCount;
   unsigned int animationCount;

   unsigned int posID, normID, colorID, uvID,
                tanID, bitanID, indID,
                texID, bIndID, bWeightID;

   int hasNormals, hasColors, hasTexCoords, hasTextures,
       hasTansAndBitans, hasBones, hasAnimations;

   short boneRoot;
   Bone * bones;
   Animation * animations;
};

#endif // __MODEL_H__
