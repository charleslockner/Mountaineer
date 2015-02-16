
#include <OpenGL/gl.h>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "safe_gl.h"
#include "model.h"
#include "attachment_loader.h"

#define MAX_INFLUENCES 15

typedef struct {
   unsigned int index;
   float weight;
} BoneWeight;

typedef struct Vertex {
   std::vector<BoneWeight> boneWeights;
} Vertex;

static bool weightCompare(BoneWeight a, BoneWeight b) {
   return a.weight > b.weight;
}

static void fillVertexArray(std::vector<float> & inWeights, std::vector<Vertex> & outVerts) {
   int boneCount = inWeights.size() / outVerts.size();

   // put each bone weight into each vertex's boneWeights list
   for (int i = 0; i < inWeights.size(); i++) {
      int boneNum = i % boneCount;
      int vertNdx = i / boneCount;

      BoneWeight bw;
      bw.index = boneNum;
      bw.weight = inWeights[i];

      outVerts[vertNdx].boneWeights.push_back(bw);
   }
}

static void sortBoneWeights(std::vector<Vertex> & verts) {
   // sort the bone indices by their bone weights (in decending order)
   for (int i = 0; i < verts.size(); i++)
      std::sort(verts[i].boneWeights.begin(), verts[i].boneWeights.end(), weightCompare);
}

static void normalizeBoneWeights(std::vector<Vertex> & verts) {
   // Make sum of each vertex's weights equal to 1
   for (int i = 0; i < verts.size(); i++) {
      float preSum = 0;
      for (int j = 0; j < MAX_INFLUENCES; j++)
         preSum += verts[i].boneWeights[j].weight;
      for (int j = 0; j < MAX_INFLUENCES; j++)
         verts[i].boneWeights[j].weight = verts[i].boneWeights[j].weight / preSum;
   }
}

static void parseVertexWeights(std::vector<Vertex> & verts, float * bIndices, float * bWeights, float * bNumInfluences) {
   // Sort vertex components into buffers to send to the GPU
   for (int i = 0; i < verts.size(); i++) {
      bool foundNum = false;

      for (int j = 0; j < MAX_INFLUENCES; j++) {
         bIndices[MAX_INFLUENCES * i + j] = float(verts[i].boneWeights[j].index);
         bWeights[MAX_INFLUENCES * i + j] = verts[i].boneWeights[j].weight;

         if (verts[i].boneWeights[j].weight == 0.0f && !foundNum) {
            bNumInfluences[i] = j;
            foundNum = true;
         }
      }

      if (!foundNum)
         bNumInfluences[i] = MAX_INFLUENCES;
   }
}

static void bindBoneWeights(Model * model, std::vector<float> & inWeights, int numBones) {
   int numVertices = inWeights.size() / numBones;
   std::vector<Vertex> verts = std::vector<Vertex>(numVertices);
   float bIndices[numVertices * MAX_INFLUENCES];
   float bWeights[numVertices * MAX_INFLUENCES];
   float bNumInfluences[numVertices];

   fillVertexArray(inWeights, verts);
   sortBoneWeights(verts);
   normalizeBoneWeights(verts);
   parseVertexWeights(verts, bIndices, bWeights, bNumInfluences);

   glGenBuffers(1, & model->bIndID);
   glBindBuffer(GL_ARRAY_BUFFER, model->bIndID);
   glBufferData(GL_ARRAY_BUFFER, sizeof(bIndices), & bIndices[0], GL_STATIC_DRAW);

   glGenBuffers(1, & model->bWeightID);
   glBindBuffer(GL_ARRAY_BUFFER, model->bWeightID);
   glBufferData(GL_ARRAY_BUFFER, sizeof(bWeights), & bWeights[0], GL_STATIC_DRAW);

   glGenBuffers(1, & model->bNumInfID);
   glBindBuffer(GL_ARRAY_BUFFER, model->bNumInfID);
   glBufferData(GL_ARRAY_BUFFER, sizeof(bNumInfluences), & bNumInfluences[0], GL_STATIC_DRAW);

   model->boneCount = numBones;
   model->hasBoneWeights = true;
   model->maxInfluences = MAX_INFLUENCES;
}

static void setBindPoseMatrices(Model * model, std::vector<float> & inBindPoses, int numBones) {
   model->bones = (Bone *)malloc(sizeof(Bone) * numBones);

   for (int i = 0; i < numBones; i++) {
      glm::vec3 tns = glm::vec3(inBindPoses[7*i+4], inBindPoses[7*i+5], inBindPoses[7*i+6]);
      glm::quat rot = glm::quat(inBindPoses[7*i], inBindPoses[7*i+1], inBindPoses[7*i+2], inBindPoses[7*i+3]);

      glm::mat4 tnsM = glm::translate(glm::mat4(1.0), tns);
      glm::mat4 rotM = glm::toMat4(rot);
      glm::mat4 bindPose = tnsM * rotM;

      model->bones[i].invBonePose = glm::inverse(bindPose);
   }
}

static void setAnimFrames(Model * model, std::vector<float> & inFrames, int numBones) {
   model->animations = (Animation *)malloc(sizeof(Animation) * 1);
   Animation * anim = & model->animations[0];

   int frameCount = inFrames.size() / (7 * numBones);
   int fps = 60;

   anim->fps = fps;
   anim->keyCount = frameCount;
   anim->duration = 1.0 * (anim->keyCount-1) / anim->fps;

   anim->animBones = (AnimBone *)malloc(sizeof(AnimBone) * numBones);

   for (int boneNdx = 0; boneNdx < numBones; boneNdx++) {
      AnimBone * animBone = & anim->animBones[boneNdx];

      animBone->keys = (Key *)malloc(sizeof(Key) * frameCount);
      for (int frameNdx = 0; frameNdx < frameCount; frameNdx++) {
         Key * key = & animBone->keys[frameNdx];

         int inNdx = (7 * numBones * frameNdx) + (7 * boneNdx);

         key->time = 1.0 * frameNdx / fps;
         key->position = glm::vec3(inFrames[inNdx+4], inFrames[inNdx+5], inFrames[inNdx+6]);
         key->rotation = glm::quat(inFrames[inNdx], inFrames[inNdx+1], inFrames[inNdx+2], inFrames[inNdx+3]);
         key->scale = glm::vec3(1, 1, 1);
      }
   }

   model->animationCount = 1;
   model->hasAnimations = true;
}

void Model::loadSkinningPIN(const char * path) {
   std::vector<float> boneWeights;
   int numBones;

   PIN_loadWeights(boneWeights, numBones, path);
   bindBoneWeights(this, boneWeights, numBones);
}

void Model::loadAnimationPIN(const char * path) {
   std::vector<float> frames;
   std::vector<float> bindPoses;
   int numBones;

   PIN_loadSkeleton(frames, bindPoses, numBones, path);
   setBindPoseMatrices(this, bindPoses, numBones);
   setAnimFrames(this, frames, numBones);
}