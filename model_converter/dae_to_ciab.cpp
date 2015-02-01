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

void writeTypeField(FILE * fp, unsigned char type) {
   fwrite(& type, sizeof(unsigned char), 1, fp);
}

void writeFloat(FILE * fp, float f) {
   fwrite(& f, sizeof(float), 1, fp);
}

void writeUInt(FILE * fp, unsigned int i) {
   fwrite(& i, sizeof(unsigned int), 1, fp);
}

void writeShort(FILE * fp, short i) {
   fwrite(& i, sizeof(short), 1, fp);
}

void writeUShort(FILE * fp, unsigned short i) {
   fwrite(& i, sizeof(unsigned short), 1, fp);
}

void writeVector3D(FILE * fp, aiVector3D v) {
   for (int i = 0; i < 3; i++)
      writeFloat(fp, v[i]);
}

void writeQuaternion(FILE * fp, aiQuaternion q) {
   writeFloat(fp, q.w);
   writeFloat(fp, q.x);
   writeFloat(fp, q.y);
   writeFloat(fp, q.z);
}

void write4x4M(FILE * fp, aiMatrix4x4 m) {
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         writeFloat(fp, m[r][c]);
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
      writeTypeField(fp, POSITIONS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, mesh.mVertices[i]);
   }
}

void writeNormals(FILE * fp, aiMesh& mesh) {
   if (mesh.HasNormals()) {
      std::cerr << "Writing vertex normals...\n";
      writeTypeField(fp, NORMALS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, mesh.mNormals[i]);
   }
}

void writeColors(FILE * fp, aiMesh& mesh) {
   if (mesh.HasVertexColors(0)) {
      std::cerr << "Writing vertex colors...\n";
      writeTypeField(fp, COLORS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         writeFloat(fp, mesh.mColors[0][i].r);
         writeFloat(fp, mesh.mColors[0][i].g);
         writeFloat(fp, mesh.mColors[0][i].b);
      }
   }
}

void writeUVs(FILE * fp, aiMesh& mesh) {
   if (mesh.HasTextureCoords(0)) {
      std::cerr << "Writing texture coordinates...\n";
      writeTypeField(fp, TEXCOORDS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         writeFloat(fp, mesh.mTextureCoords[0][i].x);
         writeFloat(fp, mesh.mTextureCoords[0][i].y);
      }
   }
}

void writeTangentsAndBitangents(FILE * fp, aiMesh& mesh) {
   if (mesh.HasTangentsAndBitangents()) {
      std::cerr << "Writing tangents...\n";
      writeTypeField(fp, TANGENTS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, mesh.mTangents[i]);

      std::cerr << "Writing bitangents...\n";
      writeTypeField(fp, BITANGENTS_TYPE);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, mesh.mBitangents[i]);
   }
}

void writeIndices(FILE * fp, aiMesh& mesh) {
   if (mesh.HasFaces()) {
      std::cerr << "Writing indices...\n";
      writeTypeField(fp, INDICES_TYPE);

      for (int i = 0; i < mesh.mNumFaces; i++) {
         writeUInt(fp, mesh.mFaces[i].mIndices[0]);
         writeUInt(fp, mesh.mFaces[i].mIndices[1]);
         writeUInt(fp, mesh.mFaces[i].mIndices[2]);
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
   writeTypeField(fp, BONE_INDICES_TYPE);

   for (uint i = 0; i < mesh.mNumVertices; i++)
      for (uint j = 0; j < MAX_INFLUENCES; j++)
         writeUShort(fp, j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].index : 0);

   std::cerr << "Writing bone weights...\n";
   writeTypeField(fp, BONE_WEIGHTS_TYPE);

   for (uint i = 0; i < mesh.mNumVertices; i++)
      for (uint j = 0; j < MAX_INFLUENCES; j++)
         writeFloat(fp, j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].weight : 0);
}

int findRootBoneIndex(aiMesh& mesh, BoneMap * n2I, aiNode * root) {
   for (uint i = 0; i < mesh.mNumBones; i++) {
      aiNode * node = root->FindNode(mesh.mBones[i]->mName);
      BoneIterator it = n2I->find(node->mParent->mName.C_Str());
      if (it == n2I->end())
         return i;
   }

   assert(false); // didn't find the root bone
   return -1;
}

BoneMap createName2IndexMap(aiMesh& mesh) {
   BoneMap nameToIndexMap;
   for (uint i = 0; i < mesh.mNumBones; i++)
      nameToIndexMap.insert(BoneMap::value_type(mesh.mBones[i]->mName.C_Str(), i));
   return nameToIndexMap;
}

void writeBoneTree(FILE * fp, aiMesh& mesh, aiNode * root) {
   std::cerr << "Writing bone tree...\n";
   writeTypeField(fp, BONE_TREE_TYPE);

   BoneMap nameToIndexMap = createName2IndexMap(mesh);

   // write the bone root index
   writeShort(fp, findRootBoneIndex(mesh, & nameToIndexMap, root));

   // Write each bone to file
   for (uint i = 0; i < mesh.mNumBones; i++) {
      aiNode * node = root->FindNode(mesh.mBones[i]->mName);

      // Write the parent index
      BoneIterator it = nameToIndexMap.find(node->mParent->mName.C_Str());
      short parentIndex = -1;
      if (it != nameToIndexMap.end())
         parentIndex = it->second;
      writeShort(fp, parentIndex);

      // Write the children
      writeShort(fp, node->mNumChildren);
      for (int j = 0; j < node->mNumChildren; j++) {
         BoneIterator it = nameToIndexMap.find(node->mChildren[j]->mName.C_Str());
         assert (it != nameToIndexMap.end());
         writeShort(fp, it->second);
      }

      // Write the inverse bindPose and parentBone transform matrices
      write4x4M(fp, mesh.mBones[i]->mOffsetMatrix);
      write4x4M(fp, node->mTransformation);
   }
}

void writeBones(FILE * fp, aiMesh& mesh, aiNode * root) {
   if (mesh.HasBones()) {
      writeBoneIndicesAndWeights(fp, mesh);
      writeBoneTree(fp, mesh, root);
   }
}

BoneMap createAnimName2IndexMap(aiAnimation * anim) {
   BoneMap nameToIndexMap;
   for (uint i = 0; i < anim->mNumChannels; i++)
      nameToIndexMap.insert(BoneMap::value_type(anim->mChannels[i]->mNodeName.C_Str(), i));
   return nameToIndexMap;
}

int getAnimIndexRoot(aiMesh& mesh, BoneMap * n2I) {
   const char * rootName = mesh.mBones[0]->mName.C_Str();
   return n2I->find(rootName)->second;
}

void writeAnimations(FILE * fp, const aiScene * scene, aiMesh& mesh) {
   aiNode * root = scene->mRootNode;
   int numAnims = scene->mNumAnimations;

   if (numAnims > 0) {
      std::cerr << "Writing " << numAnims << " animation" << (numAnims == 1 ? "" : "s") << "...\n";
      writeTypeField(fp, ANIMATIONS_TYPE);

      for (int i = 0; i < numAnims; i++) {
         aiAnimation * anim = scene->mAnimations[i];
         BoneMap nameToIndexMap = createAnimName2IndexMap(anim);
         int rootNdx = getAnimIndexRoot(mesh, & nameToIndexMap);
         assert(anim->mTicksPerSecond != 0);

         writeFloat(fp, anim->mDuration);

         for (int j = rootNdx; j < anim->mNumChannels; j++) {
            aiNodeAnim * nodeAnim = anim->mChannels[j];
            aiNode * node = root->FindNode(nodeAnim->mNodeName);

            // aiVector3D sclPI;
            // aiQuaternion rotPI;
            // aiVector3D posPI;

            // aiMatrix4x4 mPI = node->mTransformation.Inverse();
            // mPI.Decompose(sclPI, rotPI, posPI);

            writeUInt(fp, nodeAnim->mNumPositionKeys);
            for (int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
               writeFloat(fp, nodeAnim->mPositionKeys[k].mTime);
               writeVector3D(fp, nodeAnim->mPositionKeys[k].mValue);
            }

            writeUInt(fp, nodeAnim->mNumRotationKeys);
            for (int k = 0; k < nodeAnim->mNumRotationKeys; k++) {
               writeFloat(fp, nodeAnim->mRotationKeys[k].mTime);
               writeQuaternion(fp, nodeAnim->mRotationKeys[k].mValue);
            }

            writeUInt(fp, nodeAnim->mNumScalingKeys);
            for (int k = 0; k < nodeAnim->mNumScalingKeys; k++) {
               writeFloat(fp, nodeAnim->mScalingKeys[k].mTime);
               writeVector3D(fp, nodeAnim->mScalingKeys[k].mValue);
            }
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
   writeAnimations(outFile, scene, mesh);

   return 0;
}