#ifndef __MODEL_H__
#define __MODEL_H__

#include "matrix_math.h"
#include "geometry.h"
#include <vector>

#define NUM_FACE_EDGES 3
#define MAX_INFLUENCES 4
#define MAX_BONES 100
#define MAX_BONE_JOINTS 3

typedef struct Key {
   float time;
   Eigen::Vector3f position;
   Eigen::Quaternionf rotation;
   Eigen::Vector3f scale;
} Key;

typedef struct AnimBone {
   std::vector<Key> keys;
} AnimBone;

typedef struct Animation {
   unsigned int fps;
   unsigned int keyCount;
   float duration;
   std::vector<AnimBone> animBones;
} Animation;

typedef struct IKJoint {
   Eigen::Vector3f axis;
   float minAngle;
   float maxAngle;
} IKJoint;

typedef struct Bone {
   int parentIndex;
   unsigned int childCount;
   std::vector<int> childIndices;
   Eigen::Matrix4f invBonePose;
   Eigen::Matrix4f parentOffset;
   std::vector<IKJoint> joints;
} Bone;

typedef struct BoneWeight {
   unsigned int index;
   float weight;
} BoneWeight;

class Vertex;
class Face;

class Vertex {
public:
   unsigned int index;
   Eigen::Vector3f position;
   Eigen::Vector3f normal;
   Eigen::Vector3f tangent;
   Eigen::Vector3f bitangent;
   Eigen::Vector3f color;
   Eigen::Vector2f uv;
   float boneInfCount;
   float boneIndices[MAX_INFLUENCES];
   float boneWeights[MAX_INFLUENCES];

   std::vector<Vertex *> neighbors; // neighboring vertices
   std::vector<Face *> faces; // faces that use this vertex as a corner

   void calculateNormal();
   bool hasNeighbor(Vertex * n);
};

class Face {
public:
   Vertex * vertices[NUM_FACE_EDGES];
   Eigen::Vector3f normal;

   void calculateNormal();
   Eigen::Vector3f intersectRay(Geom::Rayf ray);
   bool pointCheckInside(Eigen::Vector3f pnt);
};

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
   void loadTexture(const char * path, bool repeat);
   void loadNormalMap(const char * path, bool repeat);
   void loadSpecularMap(const char * path, bool repeat);
   void loadOBJ(const char * path);
   void loadSkinningPIN(const char * path);
   void loadAnimationPIN(const char * path);
   void loadConstraints(const char * path);

   void CalculateNormals(); // Calculate vertex and face normals from vertex positions
   void bufferVertices(); // Send the vertex data to the GPU memory
   void bufferIndices(); // Send the index array to the GPU

   void printVertices();
   void printFaces();
   void printBoneTree();
   void printAnimations();

   unsigned int vertexCount, faceCount, boneCount, animationCount;

   unsigned int posID, normID, colorID, uvID, tanID, bitanID,
                indexID, texID, nmapID, smapID, bNumInfID, bIndexID, bWeightID;

   bool hasNormals, hasColors, hasTexCoords, hasTexture, hasNormalMap, hasSpecularMap,
        hasTansAndBitans, hasBoneWeights, hasBoneTree, hasAnimations, isAnimated;

   unsigned int boneRoot;
   std::vector<Vertex *> vertices;
   std::vector<Face *> faces;
   std::vector<Bone> bones;
   std::vector<Animation> animations;
};

#endif // __MODEL_H__
