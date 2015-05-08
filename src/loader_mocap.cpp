
#include "safe_gl.h"
#include "model.h"
#include "attachment_loader.h"

#include <algorithm>

typedef struct VertexWeight {
   std::vector<BoneWeight> boneWeights;
} VertexWeight;

static bool weightCompare(BoneWeight a, BoneWeight b) {
   return a.weight > b.weight;
}

static void fillVertexArray(std::vector<float> & inWeights, std::vector<VertexWeight> & outVerts) {
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

static void sortBoneWeights(std::vector<VertexWeight> & verts) {
   // sort the bone indices by their bone weights (in decending order)
   for (int i = 0; i < verts.size(); i++)
      std::sort(verts[i].boneWeights.begin(), verts[i].boneWeights.end(), weightCompare);
}

static void normalizeBoneWeights(std::vector<VertexWeight> & verts) {
   // Make sum of each vertex's weights equal to 1
   for (int i = 0; i < verts.size(); i++) {
      float preSum = 0;
      for (int j = 0; j < MAX_INFLUENCES; j++)
         preSum += verts[i].boneWeights[j].weight;
      for (int j = 0; j < MAX_INFLUENCES; j++)
         verts[i].boneWeights[j].weight = verts[i].boneWeights[j].weight / preSum;
   }
}

static void parseVertexWeights(std::vector<VertexWeight> & verts, float * bIndices, float * bWeights, float * bNumInfluences) {
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
   std::vector<VertexWeight> verts = std::vector<VertexWeight>(numVertices);
   int numWeights = numVertices * MAX_INFLUENCES;
   float * bNumInfluences = (float *)malloc(numVertices * sizeof(float));
   float * bIndices = (float *)malloc(numWeights * sizeof(float));
   float * bWeights = (float *)malloc(numWeights * sizeof(float));

   fillVertexArray(inWeights, verts);
   sortBoneWeights(verts);
   normalizeBoneWeights(verts);
   parseVertexWeights(verts, bIndices, bWeights, bNumInfluences);

   for (int i = 0; i < numVertices; i++)
      model->vertices[i]->boneInfCount = bNumInfluences[i];

   for (int i = 0; i < numVertices; i++)
      for (int j = 0; j < MAX_INFLUENCES; j++)
         model->vertices[i]->boneIndices[j] = bIndices[MAX_INFLUENCES*i+j];

   for (int i = 0; i < numVertices; i++)
      for (int j = 0; j < MAX_INFLUENCES; j++)
         model->vertices[i]->boneWeights[j] = bWeights[MAX_INFLUENCES*i+j];

   model->boneCount = numBones;
   model->hasBoneWeights = true;

   model->bufferVertices();

   free(bIndices);
   free(bWeights);
   free(bNumInfluences);
}

static void setBindPoseMatrices(Model * model, std::vector<float> & inBindPoses, int numBones) {
   model->bones = std::vector<Bone>(numBones);

   for (int i = 0; i < numBones; i++) {
      Eigen::Vector3f tns = Eigen::Vector3f(inBindPoses[7*i+4], inBindPoses[7*i+5], inBindPoses[7*i+6]);
      Eigen::Quaternionf rot = Eigen::Quaternionf(inBindPoses[7*i+3], inBindPoses[7*i], inBindPoses[7*i+1], inBindPoses[7*i+2]);

      Eigen::Matrix4f tnsM = Mmath::TranslationMatrix(tns);
      Eigen::Matrix4f rotM = Mmath::RotationMatrix(rot);
      Eigen::Matrix4f bindPose = tnsM * rotM;

      model->bones[i].invBonePose = bindPose.inverse();
   }
}

static void setAnimFrames(Model * model, std::vector<float> & inFrames, int numBones) {
   model->animations = std::vector<Animation>(1);
   Animation * anim = & model->animations[0];

   int keyCount = inFrames.size() / (7 * numBones);
   int fps = 60;

   anim->fps = fps;
   anim->keyCount = keyCount;
   anim->duration = 1.0 * (keyCount-1) / fps;

   anim->animBones = std::vector<AnimBone>(numBones);
   for (int boneNdx = 0; boneNdx < numBones; boneNdx++) {
      AnimBone * animBone = & anim->animBones[boneNdx];

      animBone->keys = std::vector<Key>(anim->keyCount);
      for (int keyNdx = 0; keyNdx < keyCount; keyNdx++) {
         Key * key = & animBone->keys[keyNdx];

         int inNdx = (7 * numBones * keyNdx) + (7 * boneNdx);

         key->time = 1.0 * keyNdx / fps;
         key->position = Eigen::Vector3f(inFrames[inNdx+4], inFrames[inNdx+5], inFrames[inNdx+6]);
         key->rotation = Eigen::Quaternionf(inFrames[inNdx+3], inFrames[inNdx], inFrames[inNdx+1], inFrames[inNdx+2]);
         key->scale = Eigen::Vector3f(1, 1, 1);
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

   isAnimated = hasBoneWeights && hasAnimations;
}

void Model::loadAnimationPIN(const char * path) {
   std::vector<float> frames;
   std::vector<float> bindPoses;
   int numBones;

   PIN_loadSkeleton(frames, bindPoses, numBones, path);
   setBindPoseMatrices(this, bindPoses, numBones);
   setAnimFrames(this, frames, numBones);

   isAnimated = hasBoneWeights && hasAnimations;
}
