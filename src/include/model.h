#ifndef __MODEL_H__
#define __MODEL_H__

#include "matrix_math.h"
#include <vector>

#define NUM_FACE_EDGES 3
#define MAX_INFLUENCES 4
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
   std::vector<AnimBone> animBones;
} Animation;

typedef struct {
   Eigen::Vector3f axis;
   float minAngle;
   float maxAngle;
   int boneIndex;
} IKJoint;

typedef struct {
   std::vector<IKJoint> joints;
} IKLimb;

typedef struct {
   short parentIndex;
   short childCount;
   std::vector<short> childIndices;
   Eigen::Matrix4f invBonePose;
   Eigen::Matrix4f parentOffset;
   IKLimb limb;
   IKJoint * joint;
} Bone;

typedef struct {
   Eigen::Vector3f position;
   Eigen::Vector3f tangent;
   Eigen::Vector3f bitangent;
   Eigen::Vector3f normal;
   Eigen::Vector3f color;
   Eigen::Vector2f uv;
   unsigned int boneIndices[MAX_BONES];
   float boneWeights[MAX_BONES];
   unsigned int boneInfluencesCount;
} Vertex;

typedef struct {
   unsigned int vertices[NUM_FACE_EDGES];
} Face;

typedef struct {
   Face * face;
   Face * expandedFace;
   unsigned char indexOfFaceVertices; // 0, 1, or 2
} FaceUpdate;

typedef struct {
   Vertex * emergeVertex;
   Vertex * baseVertex;
   std::vector<Face *> emergingFaces;
   std::vector<FaceUpdate> faceUpdates;
} TesselateStep;

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

   unsigned int vertexCount, faceCount, boneCount, animationCount;

   unsigned int posID, normID, colorID, uvID, tanID, bitanID,
                indID, texID, nmapID, smapID, bIndID, bWeightID, bNumInfID;

   bool hasNormals, hasColors, hasTexCoords, hasTexture, hasNormalMap, hasSpecularMap,
        hasTansAndBitans, hasBoneWeights, hasBoneTree, hasAnimations, isAnimated;

   short boneRoot;
   std::vector<Vertex> vertices;
   std::vector<Face> faces;
   std::vector<Bone> bones;
   std::vector<Animation> animations;
};

#endif // __MODEL_H__
