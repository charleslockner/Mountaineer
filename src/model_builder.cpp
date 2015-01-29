
#include <OpenGL/gl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "model_builder.h"

#define MAX_INFLUENCES 4

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
   BONE_TREE = 10,
   ANIMATIONS = 11
} modelFieldType;

static void readHeader(FILE *fp, Model * model) {
   fread(& model->vertexCount, sizeof(unsigned int), 1, fp);
   fread(& model->indexCount, sizeof(unsigned int), 1, fp);
   fread(& model->boneCount, sizeof(unsigned int), 1, fp);
   fread(& model->animationCount, sizeof(unsigned int), 1, fp);

   if (model->boneCount > MAX_BONES) {
      printf("There are %d bones and the max is %d\n", model->boneCount, MAX_BONES);
      exit(1);
   }
}

static unsigned int createVBO(FILE *fp, int count, int size) {
   void * data = (void *)malloc(count * size);
   fread(data, size, count, fp);

   unsigned int vbo;
   glGenBuffers(1, & vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

   free(data);

   return vbo;
}

static void readPositions(FILE *fp, Model * model) {
   model->vbo = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readNormals(FILE *fp, Model * model) {
   model->nbo = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readColors(FILE *fp, Model * model) {
   model->cbo = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readTexCoords(FILE *fp, Model * model) {
   model->uvbo = createVBO(fp, 2 * model->vertexCount, sizeof(float));
}

static void readTangents(FILE *fp, Model * model) {
   model->tabo = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readBitangents(FILE *fp, Model * model) {
   model->btbo = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readIndices(FILE *fp, Model * model) {
   model->ibo = createVBO(fp, model->indexCount, sizeof(unsigned int));
}

static void readBoneIndices(FILE *fp, Model * model) {
   int count = MAX_INFLUENCES * model->vertexCount;

   unsigned short * ushortData = (unsigned short *)malloc(count * sizeof(unsigned short));
   fread(ushortData, sizeof(unsigned short), count, fp);

   // convert the data from unsigned shorts to floats
   float * floatData = (float *)malloc(count * sizeof(float));
   for (int i = 0; i < count; i++)
      floatData[i] = ushortData[i];

   // generate the buffer
   glGenBuffers(1, & model->bibo);
   glBindBuffer(GL_ARRAY_BUFFER, model->bibo);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), floatData, GL_STATIC_DRAW);

   // free the data blocks
   free(ushortData);
   free(floatData);
}

static void readBoneWeights(FILE *fp, Model * model) {
   model->bwbo = createVBO(fp, MAX_INFLUENCES * model->vertexCount, sizeof(float));
}

static void readBoneTree(FILE *fp, Model * model) {
   fread(& model->boneRoot, sizeof(short), 1, fp);

   model->bones = (Bone *)malloc(model->boneCount * sizeof(Bone));

   for (int i = 0; i < model->boneCount; i++) {
      Bone * bone = & model->bones[i];
      fread(& bone->parentIndex, sizeof(short), 1, fp);
      fread(& bone->childCount, sizeof(short), 1, fp);
      bone->childIndices = (short *)malloc(bone->childCount * sizeof(short));
      for (int j = 0; j < bone->childCount; j++)
         fread(& bone->childIndices[j], sizeof(short), 1, fp);
      fread(& bone->invBonePose, sizeof(glm::mat4), 1, fp);
      bone->invBonePose = glm::transpose(bone->invBonePose);
      fread(& bone->parentOffset, sizeof(glm::mat4), 1, fp);
      bone->parentOffset = glm::transpose(bone->parentOffset);
   }
}

// for some reason glm assumes w is the last entry, not the first as the data is usually given
static glm::quat correctQuatW(glm::quat q) {
   return glm::quat(q.x, q.y, q.z, q.w);
}

static void readAnimations(FILE *fp, Model * model) {
   model->animations = (Animation *)malloc(model->animationCount * sizeof(Animation));

   for (int i = 0; i < model->animationCount; i++) {
      Animation * anim = & model->animations[i];

      fread(& anim->duration, sizeof(float), 1, fp);
      anim->animBones = (AnimBone *)malloc(model->boneCount * sizeof(AnimBone));

      for (int j = 0; j < model->boneCount; j++) {
         AnimBone * animBone = & anim->animBones[j];

         fread(& animBone->translateKeyCount, sizeof(unsigned int), 1, fp);
         animBone->translateKeys = (Vec3Key *)malloc(animBone->translateKeyCount * sizeof(Vec3Key));
         for (int k = 0; k < animBone->translateKeyCount; k++) {
            Vec3Key * key = & animBone->translateKeys[k];
            fread(& key->time, sizeof(int), 1, fp);
            fread(& key->value, sizeof(glm::vec3), 1, fp);
         }

         fread(& animBone->rotateKeyCount, sizeof(unsigned int), 1, fp);
         animBone->rotateKeys = (QuatKey *)malloc(animBone->rotateKeyCount * sizeof(QuatKey));
         for (int k = 0; k < animBone->rotateKeyCount; k++) {
            QuatKey * key = & animBone->rotateKeys[k];
            fread(& key->time, sizeof(int), 1, fp);
            fread(& key->value, sizeof(glm::quat), 1, fp);
            key->value = correctQuatW(key->value);
         }

         fread(& animBone->scaleKeyCount, sizeof(unsigned int), 1, fp);
         animBone->scaleKeys = (Vec3Key *)malloc(animBone->scaleKeyCount * sizeof(Vec3Key));
         for (int k = 0; k < animBone->scaleKeyCount; k++) {
            Vec3Key * key = & animBone->scaleKeys[k];
            fread(& key->time, sizeof(int), 1, fp);
            fread(& key->value, sizeof(glm::vec3), 1, fp);
         }
      }
   }
}

static void printBoneTree(Bone * boneTree, short numBones) {
   for (int i = 0; i < numBones; i++) {
      Bone * bone = & boneTree[i];
      printf("Bone: %d\n", i);
      printf("  parent: %d\n", bone->parentIndex);
      for (int j = 0; j < bone->childCount; j++) {
         printf("  child: %d\n", bone->childIndices[j]);
      }
      glm::mat4 m = bone->invBonePose;
      printf("  invBonePose:\n");
      printf("    %.2f %.2f %.2f %.2f\n", m[0][0], m[1][0], m[2][0], m[3][0]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][1], m[1][1], m[2][1], m[3][1]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][2], m[1][2], m[2][2], m[3][2]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][3], m[1][3], m[2][3], m[3][3]);
      m = bone->parentOffset;
      printf("  parentOffset:\n");
      printf("    %.2f %.2f %.2f %.2f\n", m[0][0], m[1][0], m[2][0], m[3][0]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][1], m[1][1], m[2][1], m[3][1]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][2], m[1][2], m[2][2], m[3][2]);
      printf("    %.2f %.2f %.2f %.2f\n", m[0][3], m[1][3], m[2][3], m[3][3]);
   }
}

static void printAnimations(Animation * anims, short numAnims, short numBones) {
   for (int i = 0; i < numAnims; i++) {
      Animation * anim = & anims[i];

      printf("Animation %d:\n", i);
      printf("duration %f\n", anim->duration);

      for (int j = 0; j < numBones; j++) {
         AnimBone * animBone = & anim->animBones[j];
         printf("  AnimBone %d:\n", j);

         printf("  translateKeyCount %d\n", animBone->translateKeyCount);
         for (int k = 0; k < animBone->translateKeyCount; k++) {
            Vec3Key * key = & animBone->translateKeys[k];
            printf("    translateKey: frame %d: (%.2f, %.2f, %.2f)\n", k, key->value.x, key->value.y, key->value.z);
         }

         printf("  rotateKeyCount %d\n", animBone->rotateKeyCount);
         for (int k = 0; k < animBone->rotateKeyCount; k++) {
            QuatKey * key = & animBone->rotateKeys[k];
            printf("    rotateKey: frame %d: (%.2f, %.2f, %.2f, %.2f)\n", k, key->value.w, key->value.x, key->value.y, key->value.z);
         }

         printf("  scaleKeyCount %d\n", animBone->scaleKeyCount);
         for (int k = 0; k < animBone->scaleKeyCount; k++) {
            Vec3Key * key = & animBone->scaleKeys[k];
            printf("    scaleKey: frame %d: (%.2f, %.2f, %.2f)\n", k, key->value.x, key->value.y, key->value.z);
         }
      }
   }
}

static void loadMeshData(FILE *fp, Model * model) {
   readHeader(fp, model);

   char fieldType;
   while (fread(&fieldType, sizeof(char), 1, fp) > 0) {
      printf("Encountered %d field\n", fieldType);
      switch(fieldType) {
         case POSITIONS:
            readPositions(fp, model);
            break;
         case NORMALS:
            readNormals(fp, model);
            break;
         case COLORS:
            readColors(fp, model);
            break;
         case TEXCOORDS:
            readTexCoords(fp, model);
            break;
         case TANGENTS:
            readTangents(fp, model);
            break;
         case BITANGENTS:
            readBitangents(fp, model);
            break;
         case INDICES:
            readIndices(fp, model);
            break;
         case BONE_INDICES:
            readBoneIndices(fp, model);
            break;
         case BONE_WEIGHTS:
            readBoneWeights(fp, model);
            break;
         case BONE_TREE:
            readBoneTree(fp, model);
            break;
         case ANIMATIONS:
            readAnimations(fp, model);
            break;
         default:
            printf("Invalid model field '%d' encountered while loading model\n", fieldType);
            exit(1);
            break;
      }
   }
}

FILE * safe_fopen(const char * path) {
   if (!path) {
      fprintf(stderr, "Error reading model file. Filename string empty\n");
      exit(1);
   }

   FILE * fp = fopen(path,"rb");

   if (!fp) {
      printf("Error loading %s:\n ", path);
      perror("");
      exit(1);
   }

   return fp;
}

Model * MB_build(const char * meshPath, const char * texPath) {
   Model * model = (Model *)malloc(sizeof(Model));
   FILE *fp;

   fp = safe_fopen(meshPath);
   loadMeshData(fp, model);
   fclose(fp);

   model->tbo = MB_loadTexture(texPath);

   // printBoneTree(model->bones, model->boneCount);
   // printAnimations(model->animations, model->animationCount, model->boneCount);

   return model;
}

void MB_destroy(Model * model) {
   for (int i = 0; i < model->boneCount; i++)
      free(model->bones[i].childIndices);
   free(model->bones);

   for (int i = 0; i < model->animationCount; i++) {
      for (int j = 0; j < model->boneCount; j++) {
         free(model->animations[i].animBones[j].translateKeys);
         free(model->animations[i].animBones[j].rotateKeys);
         free(model->animations[i].animBones[j].scaleKeys);
      }
      free(model->animations[i].animBones);
   }
   free(model->animations);

   free(model);
}

