#ifndef __MODEL_H__
#define __MODEL_H__

#include "matrix_math.h"
#include <vector>

#define MAX_BONES 100

typedef struct {
   float time;
   Eigen::Vector3f position;
   Eigen::Quaternionf rotation;
   Eigen::Vector3f scale;
} Key;

typedef struct {
   std::vector<Key> keys;
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
   Eigen::Matrix4f invBonePose;
   Eigen::Matrix4f parentOffset;
} Bone;

class Model {
public:
   Model();
   ~Model();

   void loadCIAB(const char * path);
   void loadTexture(const char * path);
   void loadNormalMap(const char * path);
   void loadSpecularMap(const char * path);
   void loadOBJ(const char * path);
   void loadSkinningPIN(const char * path);
   void loadAnimationPIN(const char * path);

   void printBoneTree();
   void printAnimations();

   unsigned int vertexCount, faceCount, boneCount, animationCount, maxInfluences;

   unsigned int posID, normID, colorID, uvID, tanID, bitanID,
                indID, texID, nmapID, smapID, bIndID, bWeightID, bNumInfID;

   bool hasNormals, hasColors, hasTexCoords, hasTexture, hasNormalMap, hasSpecularMap,
        hasTansAndBitans, hasBoneWeights, hasBoneTree, hasAnimations, isAnimated;

   short boneRoot;
   Bone * bones;
   Animation * animations;
};

#endif // __MODEL_H__
