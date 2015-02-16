#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <iostream>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

typedef enum {
   POSITIONS = 1,
   NORMALS = 2,
   COLORS = 3,
   TEXCOORDS = 4,
   TANGENTS = 5,
   BITANGENTS = 6,
   INDICES = 7,
   BONE_INDICES = 8,
   BONE_WEIGHTS = 9,
   BONE_NUM_INF = 10,
   BONE_TREE = 11,
   ANIMATIONS = 12
} modelFieldType;

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

bool blenderCorrect = false;

FILE * safe_fopen(const char * path, const char * mode) {
   FILE * fp = fopen(path, mode);

   if (!fp) {
      std::cerr << "Error opening " << path << "\n";
      perror("");
      exit(EXIT_FAILURE);
   }

   return fp;
}

glm::mat4 aiToGlmMat4(aiMatrix4x4 mat) {
   return glm::mat4(
      mat.a1, mat.b1, mat.c1, mat.d1,
      mat.a2, mat.b2, mat.c2, mat.d2,
      mat.a3, mat.b3, mat.c3, mat.d3,
      mat.a4, mat.b4, mat.c4, mat.d4 );
}

aiMatrix4x4 glmToAIMat4(glm::mat4 mat) {
   return aiMatrix4x4(
      mat[0][0], mat[1][0], mat[2][0], mat[3][0],
      mat[0][1], mat[1][1], mat[2][1], mat[3][1],
      mat[0][2], mat[1][2], mat[2][2], mat[3][2],
      mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
}

glm::quat aiToGlmQuat(aiQuaternion quat) {
   return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

glm::vec3 aiToGlmVec3(aiVector3D vec) {
   return glm::vec3(vec.x, vec.y, vec.z);
}

aiVector3D glmToAIVec3(glm::vec3 vec) {
   return aiVector3D(vec.x, vec.y, vec.z);
}

aiQuaternion glmToAIQuat(glm::quat q) {
   return aiQuaternion(q.w, q.x, q.y, q.z);
}

aiVector3D blend2oglVec3(aiVector3D v) {
   return blenderCorrect ? aiVector3D(v.y, v.z, v.x) : v;
}

aiMatrix4x4 blend2oglMat4(aiMatrix4x4 m) {
   return blenderCorrect ? aiMatrix4x4(m.b2, m.b3, m.b1, m.b4,
                                       m.c2, m.c3, m.c1, m.c4,
                                       m.a2, m.a3, m.a1, m.a4,
                                       m.d2, m.d3, m.d1, m.d4) : m;
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
   writeFloat(fp, q.x);
   writeFloat(fp, q.y);
   writeFloat(fp, q.z);
   writeFloat(fp, q.w);
}

void write4x4M(FILE * fp, aiMatrix4x4 m) {
   for (int c = 0; c < 4; c++)
      for (int r = 0; r < 4; r++)
         writeFloat(fp, m[r][c]);
}

void writeHeader(FILE * fp, aiMesh& mesh, int animCount) {
   std::cerr << "Writing header...\n";

   fwrite(& mesh.mNumVertices, sizeof(int), 1, fp);
   fwrite(& mesh.mNumFaces, sizeof(int), 1, fp);
   fwrite(& mesh.mNumBones, sizeof(int), 1, fp);
   fwrite(& animCount, sizeof(int), 1, fp);

   std::cerr << "verts: " << mesh.mNumVertices << ", faces: " <<
      mesh.mNumFaces << ", bones: " << mesh.mNumBones << ", anims: " << animCount << "\n";
}

void writePositions(FILE * fp, aiMesh& mesh) {
   if (mesh.HasPositions()) {
      std::cerr << "Writing vertex positions...\n";
      writeTypeField(fp, POSITIONS);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, blend2oglVec3(mesh.mVertices[i]));
   }
}

void writeNormals(FILE * fp, aiMesh& mesh) {
   if (mesh.HasNormals()) {
      std::cerr << "Writing vertex normals...\n";
      writeTypeField(fp, NORMALS);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, blend2oglVec3(mesh.mNormals[i]));
   }
}

void writeColors(FILE * fp, aiMesh& mesh) {
   if (mesh.HasVertexColors(0)) {
      std::cerr << "Writing vertex colors...\n";
      writeTypeField(fp, COLORS);

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
      writeTypeField(fp, TEXCOORDS);

      for (int i = 0; i < mesh.mNumVertices; i++) {
         writeFloat(fp, mesh.mTextureCoords[0][i].x);
         writeFloat(fp, mesh.mTextureCoords[0][i].y);
      }
   }
}

void writeTangentsAndBitangents(FILE * fp, aiMesh& mesh) {
   if (mesh.HasTangentsAndBitangents()) {
      std::cerr << "Writing tangents...\n";
      writeTypeField(fp, TANGENTS);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, blend2oglVec3(mesh.mTangents[i]));

      std::cerr << "Writing bitangents...\n";
      writeTypeField(fp, BITANGENTS);

      for (int i = 0; i < mesh.mNumVertices; i++)
         writeVector3D(fp, blend2oglVec3(mesh.mBitangents[i]));
   }
}

void writeIndices(FILE * fp, aiMesh& mesh) {
   if (mesh.HasFaces()) {
      std::cerr << "Writing face indices...\n";
      writeTypeField(fp, INDICES);

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

void normalizeBoneWeights(Vertex * verts, int numVerts) {
   // Make sum of each vertex's weights equal to 1
   for (int i = 0; i < numVerts; i++) {
      float preSum = 0;
      for (int j = 0; j < MAX_INFLUENCES; j++)
         preSum += verts[i].boneWeights[j].weight;
      // printf("preSum = %f\n", preSum);

      for (int j = 0; j < MAX_INFLUENCES; j++)
         verts[i].boneWeights[j].weight = verts[i].boneWeights[j].weight / preSum;

      // float postSum = 0;
      // for (int j = 0; j < MAX_INFLUENCES; j++)
      //    postSum += verts[i].boneWeights[j].weight;
      // printf("postSum = %f\n", postSum);
   }
}

void writeBoneWeights(FILE * fp, aiMesh& mesh) {
   Vertex * verts = new Vertex[mesh.mNumVertices];
   arrangeBoneWeights(mesh, verts);
   // normalizeBoneWeights(verts, mesh.mNumVertices);

   std::cerr << "Writing bone indices...\n";
   writeTypeField(fp, BONE_INDICES);

   for (uint i = 0; i < mesh.mNumVertices; i++)
      for (uint j = 0; j < MAX_INFLUENCES; j++)
         writeUShort(fp, j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].index : 0);

   std::cerr << "Writing bone weights...\n";
   writeTypeField(fp, BONE_WEIGHTS);

   for (uint i = 0; i < mesh.mNumVertices; i++)
      for (uint j = 0; j < MAX_INFLUENCES; j++)
         writeFloat(fp, j < verts[i].boneWeights.size() ? verts[i].boneWeights[j].weight : 0);

   std::cerr << "Writing bone influences...\n";
   writeTypeField(fp, BONE_NUM_INF);

   for (uint i = 0; i < mesh.mNumVertices; i++) {
      bool foundNum = false;

      for (int j = 0; j < MAX_INFLUENCES; j++)
         if (verts[i].boneWeights[j].weight <= 0.0f) {
            writeUShort(fp, j);
            foundNum = true;
            break;
         }

      if (!foundNum)
         writeUShort(fp, MAX_INFLUENCES);
   }
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
   writeTypeField(fp, BONE_TREE);

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
      aiMatrix4x4 invBindPose = blend2oglMat4(mesh.mBones[i]->mOffsetMatrix);
      aiMatrix4x4 parentTrans = blend2oglMat4(node->mTransformation);
      write4x4M(fp, invBindPose);
      write4x4M(fp, parentTrans);
   }
}

void writeBones(FILE * fp, aiMesh& mesh, aiNode * root) {
   if (mesh.HasBones()) {
      writeBoneWeights(fp, mesh);
      writeBoneTree(fp, mesh, root);
   }
}

glm::mat4 prsKeysToMat4(glm::vec3 t, glm::quat r, glm::vec3 s) {
   glm::mat4 transM = glm::translate(glm::mat4(1.0), t);
   glm::mat4 rotateM = glm::toMat4(r);
   glm::mat4 scaleM = glm::scale(glm::mat4(1.0), s);

   return transM * scaleM * rotateM;
}

aiMatrix4x4 keysToMatrix(aiVector3D& p, aiQuaternion& r, aiVector3D& s) {
   return glmToAIMat4(prsKeysToMat4(aiToGlmVec3(p), aiToGlmQuat(r), aiToGlmVec3(s)));
}

void animKeysToMatrices(aiNode * root, aiNodeAnim * nodeAnim, aiMatrix4x4 * mats) {
   // aiNode * node = root->FindNode(nodeAnim->mNodeName);
   // aiMatrix4x4 invParentM = node->mTransformation.Inverse();

   for (int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
      aiVector3D& p = nodeAnim->mPositionKeys[k].mValue;
      aiQuaternion& r = nodeAnim->mRotationKeys[k].mValue;
      aiVector3D& s = nodeAnim->mScalingKeys[k].mValue;

      mats[k] = keysToMatrix(p, r, s);
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

void checkIfKeysAligned(aiNodeAnim * nodeAnim) {
   assert(nodeAnim->mNumPositionKeys == nodeAnim->mNumRotationKeys &&
          nodeAnim->mNumPositionKeys == nodeAnim->mNumScalingKeys);

   for (int k = 0; k < nodeAnim->mNumPositionKeys; k++)
      assert(nodeAnim->mPositionKeys[k].mTime == nodeAnim->mRotationKeys[k].mTime &&
             nodeAnim->mPositionKeys[k].mTime == nodeAnim->mScalingKeys[k].mTime);
}

void writeAnimations(FILE * fp, const aiScene * scene, aiMesh& mesh) {
   aiNode * root = scene->mRootNode;
   int numAnims = scene->mNumAnimations;

   if (numAnims > 0) {
      std::cerr << "Writing " << numAnims << " animation" << (numAnims == 1 ? "" : "s") << "...\n";
      writeTypeField(fp, ANIMATIONS);

      for (int i = 0; i < numAnims; i++) {
         aiAnimation * anim = scene->mAnimations[i];
         BoneMap nameToIndexMap = createAnimName2IndexMap(anim);
         int rootNdx = getAnimIndexRoot(mesh, & nameToIndexMap);
         assert(anim->mTicksPerSecond != 0);
         assert(anim->mNumChannels > 0);

         unsigned int numKeys = anim->mChannels[rootNdx]->mNumPositionKeys;
         unsigned int fps = numKeys / anim->mDuration;

         writeUInt(fp, fps);
         writeUInt(fp, numKeys);

         aiMatrix4x4 * matKeys = (aiMatrix4x4 *)malloc(sizeof(aiMatrix4x4) * numKeys);

         // Write the animations for each bone
         for (int j = rootNdx; j < anim->mNumChannels; j++) {
            aiNodeAnim * nodeAnim = anim->mChannels[j];
            checkIfKeysAligned(nodeAnim);

            animKeysToMatrices(root, nodeAnim, matKeys);

            for (int k = 0; k < numKeys; k++) {
               aiVector3D scl, pos;
               aiQuaternion rot;
               matKeys[k] = blend2oglMat4(matKeys[k]);
               matKeys[k].Decompose(scl, rot, pos);

               writeFloat(fp, nodeAnim->mPositionKeys[k].mTime);
               writeVector3D(fp, pos);
               writeQuaternion(fp, rot);
               writeVector3D(fp, scl);
            }
         }

         free(matKeys);
      }
   }
}

int main(int argc, char** argv) {
   char * modelPath, * outPath;

   // PARSE COMMAND LINE ARGS
   if (argc < 3 || argc > 4) {
      std::cerr << "Usage: [-b](opt) [dae_path] [ciab_path]\n";
      exit(1);
   } else if (argc == 3) {
      modelPath = argv[1];
      outPath = argv[2];
   } else if (strcmp(argv[1], "-b") == 0) {
      blenderCorrect = true;
      modelPath = argv[2];
      outPath = argv[3];
   } else {
      std::cerr << "\"-b\" option must be specified first if you want to correct for blender orientation\n";
      exit(1);
   }

   // SETUP ASSIMP
   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile(modelPath,
      aiProcess_CalcTangentSpace       |
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType            |
      aiProcess_LimitBoneWeights
   );
   if (!scene) {
      std::cerr << "Error: " << importer.GetErrorString() << "\nNow exiting...\n";
      exit(1);
   }

   aiMesh& mesh = *scene->mMeshes[0];
   aiNode * root = scene->mRootNode;
   int numAnims = scene->mNumAnimations;

   FILE * outFile = safe_fopen(outPath, "wb");

   // WRITE THE DATA
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