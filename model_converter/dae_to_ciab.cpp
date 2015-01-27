#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <map>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define POSITIONS_TYPE     1
#define NORMALS_TYPE       2
#define COLORS_TYPE        3
#define TEXCOORDS_TYPE     4
#define TANGENTS_TYPE      5
#define BITANGENTS_TYPE    6
#define INDICES_TYPE       7
#define BONE_INDICES_TYPE  8
#define BONE_WEIGHTS_TYPE  9
#define BONE_TREE_TYPE     10
#define ANIMATIONS_TYPE    11

#define MAX_INFLUENCES 4

typedef struct {
   unsigned int index;
   float weight;
} BoneWeight;

typedef struct {
   std::vector<BoneWeight> boneWeights;
} Vertex;

typedef std::map<std::string, uint> BoneMap;
typedef std::map<std::string, uint>::iterator BoneIterator;

FILE * safe_fopen(const char * path, const char * mode) {
   FILE * fp = fopen(path, mode);

   if (!fp) {
      printf("Error opening %s:\n ", path);
      perror("");
      exit(EXIT_FAILURE);
   }

   return fp;
}

void writeHeader(FILE * fp, aiMesh& mesh, int animCount) {
   std::cerr << "Writing header...\n";
   int numIndices = mesh.mNumFaces * 3;

   fwrite(& mesh.mNumVertices, sizeof(int), 1, fp);
   fwrite(& numIndices, sizeof(int), 1, fp);
   fwrite(& mesh.mNumBones, sizeof(int), 1, fp);
   fwrite(& animCount, sizeof(int), 1, fp);

   std::cerr << "verts: " << mesh.mNumVertices << ", indices: " <<
      numIndices << ", bones: " << mesh.mNumBones << ", anims: " << animCount << "\n";
}

void writePositions(FILE * fp, aiMesh& mesh) {
   if (mesh.HasPositions()) {
      std::cerr << "Writing vertex positions...\n";
      unsigned char type = POSITIONS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         float posX = mesh.mVertices[i][0];
         fwrite(& posX, sizeof(float), 1, fp);
         float posY = mesh.mVertices[i][1];
         fwrite(& posY, sizeof(float), 1, fp);
         float posZ = mesh.mVertices[i][2];
         fwrite(& posZ, sizeof(float), 1, fp);
      }
   }
}

void writeNormals(FILE * fp, aiMesh& mesh) {
   if (mesh.HasNormals()) {
      std::cerr << "Writing vertex normals...\n";
      unsigned char type = NORMALS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++)
         fwrite(& mesh.mNormals[i][0], sizeof(float), 3, fp);
   }
}

void writeColors(FILE * fp, aiMesh& mesh) {
   if (mesh.HasVertexColors(0)) {
      std::cerr << "Writing vertex normals...\n";
      unsigned char type = COLORS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         fwrite(& mesh.mColors[0][i].r, sizeof(float), 1, fp);
         fwrite(& mesh.mColors[0][i].g, sizeof(float), 1, fp);
         fwrite(& mesh.mColors[0][i].b, sizeof(float), 1, fp);
      }
   }
}

void writeUVs(FILE * fp, aiMesh& mesh) {
   if (mesh.HasTextureCoords(0)) {
      std::cerr << "Writing texture coordinates...\n";
      unsigned char type = TEXCOORDS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         fwrite(& mesh.mTextureCoords[0][i].x, sizeof(float), 1, fp);
         fwrite(& mesh.mTextureCoords[0][i].y, sizeof(float), 1, fp);
      }
   }
}

void writeTangentsAndBitangents(FILE * fp, aiMesh& mesh) {
   if (mesh.HasTangentsAndBitangents()) {
      std::cerr << "Writing tangents...\n";
      unsigned char type = TANGENTS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++)
         fwrite(& mesh.mTangents[i][0], sizeof(float), 3, fp);

      std::cerr << "Writing bitangents...\n";
      type = BITANGENTS_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumVertices; i++)
         fwrite(& mesh.mBitangents[i][0], sizeof(float), 3, fp);
   }
}

void writeIndices(FILE * fp, aiMesh& mesh) {
   if (mesh.HasFaces()) {
      std::cerr << "Writing indices...\n";

      unsigned char type = INDICES_TYPE;
      fwrite(& type, sizeof(unsigned char), 1, fp);

      for (int i = 0; i < mesh.mNumFaces; i++) {
         unsigned int index = mesh.mFaces[i].mIndices[0];
         fwrite(& index, sizeof(unsigned int), 1, fp);
         index = mesh.mFaces[i].mIndices[1];
         fwrite(& index, sizeof(unsigned int), 1, fp);
         index = mesh.mFaces[i].mIndices[2];
         fwrite(& index, sizeof(unsigned int), 1, fp);
         // std::cerr << mesh.mFaces[i].mIndices[0] << " " << mesh.mFaces[i].mIndices[1] << " " << mesh.mFaces[i].mIndices[2] << "\n";
      }
   }
}

bool weightCompare(BoneWeight a, BoneWeight b) {
   return a.weight > b.weight;
}

void arrangeBoneWeights(aiMesh& mesh, Vertex * verts) {
   // put each bone weight into each vertex's boneWeights list
   for (uint boneNum = 0; boneNum < mesh.mNumBones; boneNum++) {
      aiBone * bone = mesh.mBones[boneNum];

      for (uint j = 0; j < bone->mNumWeights; j++) {
         BoneWeight bw;
         bw.index = boneNum;
         bw.weight = bone->mWeights[j].mWeight;

         unsigned int vertNdx = bone->mWeights[j].mVertexId;
         verts[vertNdx].boneWeights.push_back(bw);
      }
   }

   // sort the bone indices by their weights (in decending order)
   for (uint i = 0; i < mesh.mNumVertices; i++)
      std::sort(verts[i].boneWeights.begin(), verts[i].boneWeights.end(), weightCompare);
}

void writeBoneIndicesAndWeights(FILE * fp, aiMesh& mesh) {
   Vertex * verts = new Vertex[mesh.mNumVertices];
   arrangeBoneWeights(mesh, verts);

   std::cerr << "Writing bone indices...\n";
   unsigned char type = BONE_INDICES_TYPE;
   fwrite(& type, sizeof(unsigned char), 1, fp);

   for (uint i = 0; i < mesh.mNumVertices; i++) {
      for (uint j = 0; j < MAX_INFLUENCES; j++) {
         unsigned short bIndex = j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].index : 0;
         fwrite(& bIndex, sizeof(unsigned short), 1, fp);
      }
   }

   std::cerr << "Writing bone weights...\n";
   type = BONE_WEIGHTS_TYPE;
   fwrite(& type, sizeof(unsigned char), 1, fp);

   for (uint i = 0; i < mesh.mNumVertices; i++) {
      for (uint j = 0; j < MAX_INFLUENCES; j++) {
         float bWeight = j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].weight : 0;
         fwrite(& bWeight, sizeof(float), 1, fp);
      }
   }
}

int findRootBoneIndex(aiMesh& mesh, BoneMap * n2I, aiNode * root) {
   for (uint i = 0; i < mesh.mNumBones; i++) {
      aiNode * node = root->FindNode(mesh.mBones[i]->mName);

      // Write the parent index
      BoneIterator it = n2I->find(node->mParent->mName.C_Str());
      if (it == n2I->end())
         return i;
   }

   assert(false); // didn't find the root bone
   return -1;
}

void writeBoneTree(FILE * fp, aiMesh& mesh, aiNode * root) {
   std::cerr << "Writing bone tree...\n";
   unsigned char type = BONE_TREE_TYPE;
   fwrite(& type, sizeof(unsigned char), 1, fp);

   // Fill in the name to index table
   BoneMap nameToIndexMap;
   for (uint i = 0; i < mesh.mNumBones; i++)
      nameToIndexMap.insert(BoneMap::value_type(mesh.mBones[i]->mName.C_Str(), i));

   // write the bone root index
   short boneRoot = findRootBoneIndex(mesh, & nameToIndexMap, root);
   fwrite(& boneRoot, sizeof(short), 1, fp);

   // Write each bone to file
   for (uint i = 0; i < mesh.mNumBones; i++) {
      aiNode * node = root->FindNode(mesh.mBones[i]->mName);

      // Write the parent index
      BoneIterator it = nameToIndexMap.find(node->mParent->mName.C_Str());
      short parentIndex = -1;
      if (it != nameToIndexMap.end())
         parentIndex = it->second;
      fwrite(& parentIndex, sizeof(short), 1, fp);

      // Write the number of children
      short numChildren = node->mNumChildren;
      fwrite(& numChildren, sizeof(short), 1, fp);

      // Write each child index
      for (int j = 0; j < numChildren; j++) {
         BoneIterator it = nameToIndexMap.find(node->mChildren[j]->mName.C_Str());
         assert (it != nameToIndexMap.end());
         short childIndex = it->second;
         fwrite(& childIndex, sizeof(short), 1, fp);
      }

      // Write the offset matrix
      aiMatrix4x4 m = mesh.mBones[i]->mOffsetMatrix;
      for (int r = 0; r < 4; r++)
         for (int c = 0; c < 4; c++) {
            float val = m[r][c];
            fwrite(& val, sizeof(float), 1, fp);
         }
   }
}

void writeBones(FILE * fp, aiMesh& mesh, aiNode * root) {
   if (mesh.HasBones()) {
      writeBoneIndicesAndWeights(fp, mesh);
      writeBoneTree(fp, mesh, root);
   }
}

void writeAnimations(FILE * fp, const aiScene * scene) {
   unsigned char type = ANIMATIONS_TYPE;
   fwrite(& type, sizeof(unsigned char), 1, fp);

   for (int i = 0; i < scene->mNumAnimations; i++) {
      std::cerr << "Writing animation " << i << "...\n";

      aiAnimation * anim = scene->mAnimations[i];

      assert(anim->mTicksPerSecond != 0);

      float duration = anim->mDuration;
      fwrite(& duration, sizeof(float), 1, fp);

      for (int j = 0; j < anim->mNumChannels; j++) {
         aiNodeAnim * nodeAnim = anim->mChannels[j];

         fwrite(& nodeAnim->mNumPositionKeys, sizeof(unsigned int), 1, fp);
         for (int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
            float time = nodeAnim->mPositionKeys[k].mTime;
            float values[] = {nodeAnim->mPositionKeys[k].mValue.x,
                              nodeAnim->mPositionKeys[k].mValue.y,
                              nodeAnim->mPositionKeys[k].mValue.z};
            fwrite(& time, sizeof(float), 1, fp);
            fwrite(& values, sizeof(float), 3, fp);
         }

         fwrite(& nodeAnim->mNumRotationKeys, sizeof(unsigned int), 1, fp);
         for (int k = 0; k < nodeAnim->mNumRotationKeys; k++) {
            float time = nodeAnim->mRotationKeys[k].mTime;
            float values[] = {nodeAnim->mRotationKeys[k].mValue.w,
                              nodeAnim->mRotationKeys[k].mValue.x,
                              nodeAnim->mRotationKeys[k].mValue.y,
                              nodeAnim->mRotationKeys[k].mValue.z};
            fwrite(& time, sizeof(float), 1, fp);
            fwrite(& values, sizeof(float), 4, fp);
         }

         fwrite(& nodeAnim->mNumScalingKeys, sizeof(unsigned int), 1, fp);
         for (int k = 0; k < nodeAnim->mNumScalingKeys; k++) {
            float time = nodeAnim->mScalingKeys[k].mTime;
            float values[] = {nodeAnim->mScalingKeys[k].mValue.x,
                              nodeAnim->mScalingKeys[k].mValue.y,
                              nodeAnim->mScalingKeys[k].mValue.z};
            fwrite(& time, sizeof(float), 1, fp);
            fwrite(& values, sizeof(float), 3, fp);
         }
      }
   }
}

int main(int argc, char** argv) {
   char * modelPath = argv[1];
   char * outPath = argv[2];

   AI_CONFIG_PP_LBW_MAX_WEIGHTS;

   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile(modelPath,
      aiProcess_CalcTangentSpace       |
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType            |
      aiProcess_LimitBoneWeights
   );

   aiMesh& mesh = *scene->mMeshes[0];
   aiNode * root = scene->mRootNode;
   int numAnims = scene->mNumAnimations;

   FILE * outFile = safe_fopen(outPath, "wb");

   writeHeader(outFile, mesh, numAnims);
   writePositions(outFile, mesh);
   writeNormals(outFile, mesh);
   writeColors(outFile, mesh);
   writeUVs(outFile, mesh);
   writeTangentsAndBitangents(outFile, mesh);
   writeIndices(outFile, mesh);
   writeBones(outFile, mesh, root);
   writeAnimations(outFile, scene);

   return 0;
}