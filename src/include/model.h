#ifndef __MODEL_H__
#define __MODEL_H__

#include "matrix_math.h"
#include <vector>

#define NUM_FACE_EDGES 3
#define MAX_INFLUENCES 4
#define MAX_BONES 100
#define MAX_BONE_JOINTS 3

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
} IKJoint;

typedef struct {
   short parentIndex;
   short childCount;
   std::vector<short> childIndices;
   Eigen::Matrix4f invBonePose;
   Eigen::Matrix4f parentOffset;
   std::vector<IKJoint> joints;
} Bone;

typedef struct {
   unsigned int index;
   float weight;
} BoneWeight;

typedef struct {
   Eigen::Vector3f position;
   Eigen::Vector3f normal;
   Eigen::Vector3f tangent;
   Eigen::Vector3f bitangent;
   Eigen::Vector3f color;
   Eigen::Vector2f uv;
   float boneInfluencesCount;
   float boneIndices[MAX_INFLUENCES];
   float boneWeights[MAX_INFLUENCES];
} Vertex;

typedef struct {
   unsigned int vertIndices[NUM_FACE_EDGES];
} Face;

// typedef struct {
//    Face * face;
//    Face * expandedFace;
//    unsigned char indexOfFaceVertices; // 0, 1, or 2
// } FaceUpdate;

// typedef struct {
//    Vertex * emergeVertex;
//    Vertex * baseVertex;
//    std::vector<Face *> emergingFaces;
//    std::vector<FaceUpdate> faceUpdates;
// } TesselateStep;

class Model {
public:
   Model();
   ~Model();

   void loadVBV(const char * path);
   void loadCIAB(const char * path);
   void loadTexture(const char * path);
   void loadNormalMap(const char * path);
   void loadSpecularMap(const char * path);
   void loadOBJ(const char * path);
   void loadSkinningPIN(const char * path);
   void loadAnimationPIN(const char * path);
   void loadConstraints(const char * path);

   void bufferVertices(); // Send the vertex data to the GPU memory
   void bufferIndices(); // Send the index array to the GPU

   void printBoneTree();
   void printAnimations();

   unsigned int vertexCount, faceCount, boneCount, animationCount;

   unsigned int vertexID, indexID, texID, nmapID, smapID;

   bool hasNormals, hasColors, hasTexCoords, hasTexture, hasNormalMap, hasSpecularMap,
        hasTansAndBitans, hasBoneWeights, hasBoneTree, hasAnimations, isAnimated;

   short boneRoot;
   std::vector<Vertex> vertices;
   std::vector<Face> faces;
   std::vector<Bone> bones;
   std::vector<Animation> animations;
};

#endif // __MODEL_H__
