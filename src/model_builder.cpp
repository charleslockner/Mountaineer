
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

   printf("verts: %d, indices: %d, bones: %d, anims: %d\n",
      model->vertexCount, model->indexCount, model->boneCount, model->animationCount);

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
   // float * data = (float *)malloc(3 * model->vertexCount * sizeof(float));
   // fread(data, sizeof(float), 3 * model->vertexCount, fp);

   // for (int i = 0 ; i < model->vertexCount; i++)
   //    printf("pos: %f %f %f\n", data[3*i], data[3*i+1], data[3*i+2]);

   // glGenBuffers(1, & model->posID);
   // glBindBuffer(GL_ARRAY_BUFFER, model->posID);
   // glBufferData(GL_ARRAY_BUFFER, 3 * model->vertexCount * sizeof(float), data, GL_STATIC_DRAW);

   // free(data);

   model->posID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readNormals(FILE *fp, Model * model) {
   model->normID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readColors(FILE *fp, Model * model) {
   model->colorID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readTexCoords(FILE *fp, Model * model) {
   model->uvID = createVBO(fp, 2 * model->vertexCount, sizeof(float));
}

static void readTangents(FILE *fp, Model * model) {
   model->tanID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readBitangents(FILE *fp, Model * model) {
   model->bitanID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
}

static void readIndices(FILE *fp, Model * model) {
   // unsigned int * data = (unsigned int *)malloc(model->indexCount * sizeof(unsigned int));
   // fread(data, sizeof(unsigned int), model->indexCount, fp);

   // for (int i = 0 ; i < model->indexCount / 3; i++)
   //    printf("indices: %d %d %d\n", data[3*i], data[3*i+1], data[3*i+2]);

   // glGenBuffers(1, & model->posID);
   // glBindBuffer(GL_ARRAY_BUFFER, model->posID);
   // glBufferData(GL_ARRAY_BUFFER, model->indexCount * sizeof(unsigned int), data, GL_STATIC_DRAW);

   // free(data);

   model->indID = createVBO(fp, model->indexCount, sizeof(unsigned int));
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
   glGenBuffers(1, & model->bIndID);
   glBindBuffer(GL_ARRAY_BUFFER, model->bIndID);
   glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), floatData, GL_STATIC_DRAW);

   // free the data blocks
   free(ushortData);
   free(floatData);
}

static void readBoneWeights(FILE *fp, Model * model) {
   model->bWeightID = createVBO(fp, MAX_INFLUENCES * model->vertexCount, sizeof(float));
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
      fread(& bone->parentOffset, sizeof(glm::mat4), 1, fp);
   }
}

static void readAnimations(FILE *fp, Model * model) {
   model->animations = (Animation *)malloc(model->animationCount * sizeof(Animation));

   for (int i = 0; i < model->animationCount; i++) {
      Animation * anim = & model->animations[i];

      fread(& anim->fps, sizeof(unsigned int), 1, fp);
      fread(& anim->keyCount, sizeof(unsigned int), 1, fp);
      anim->duration = 1.0 * (anim->keyCount-1) / anim->fps;

      anim->animBones = (AnimBone *)malloc(model->boneCount * sizeof(AnimBone));
      for (int j = 0; j < model->boneCount; j++) {
         AnimBone * animBone = & anim->animBones[j];

         animBone->keys = (Key *)malloc(anim->keyCount * sizeof(Key));
         for (int k = 0; k < anim->keyCount; k++) {
            Key * key = & animBone->keys[k];

            fread(& key->time, sizeof(float), 1, fp);
            fread(& key->position, sizeof(float), 3, fp);
            fread(& key->rotation, sizeof(float), 4, fp);
            fread(& key->scale, sizeof(float), 3, fp);
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
      printf("fps %d\n", anim->fps);
      printf("keyCount %d\n", anim->keyCount);

      for (int j = 0; j < numBones; j++) {
         AnimBone * animBone = & anim->animBones[j];
         printf("  AnimBone %d:\n", j);

         for (int k = 0; k < anim->keyCount; k++) {
            Key * key = & animBone->keys[k];
            printf("    key %d at time %f:\n", k, key->time);
            printf("      trans: %.2f %.2f %.2f\n", key->position.x, key->position.y, key->position.z);
            printf("      rotate: %.2f %.2f %.2f %.2f\n", key->rotation.w, key->rotation.x, key->rotation.y, key->rotation.z);
            printf("      scale: %.2f %.2f %.2f\n", key->scale.x, key->scale.y, key->scale.z);
         }
      }
   }
}

static bool isFieldPresent(unsigned short flags, int field) {
   return flags & (1 << (field-1));
}

static void checkPresentFields(Model * model, unsigned short flags) {
   if (!isFieldPresent(flags, POSITIONS) || !isFieldPresent(flags, INDICES)) {
      fprintf(stderr, "Model file does not have positions and/or indices\n");
      exit(1);
   }

   model->hasNormals = isFieldPresent(flags, NORMALS);
   model->hasColors = isFieldPresent(flags, COLORS);
   model->hasTexCoords = isFieldPresent(flags, TEXCOORDS);
   model->hasTansAndBitans = isFieldPresent(flags, TANGENTS) &&
                             isFieldPresent(flags, BITANGENTS);
   model->hasBones = isFieldPresent(flags, BONE_INDICES) &&
                     isFieldPresent(flags, BONE_WEIGHTS) &&
                     isFieldPresent(flags, BONE_TREE);
   model->hasAnimations = isFieldPresent(flags, ANIMATIONS);
   printf("PresentFields: %d %d %d %d %d %d\n", model->hasNormals, model->hasColors, model->hasTexCoords, model->hasTansAndBitans, model->hasBones, model->hasAnimations);
}

static void loadMeshData(FILE *fp, Model * model) {
   readHeader(fp, model);

   char fieldType;
   unsigned short receivedFlags = 0;

   while (fread(&fieldType, sizeof(char), 1, fp) > 0) {
      receivedFlags |= 1 << (fieldType-1);

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

      printf("Loading field %d\n", fieldType);
   }

   checkPresentFields(model, receivedFlags);
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

   model->texID = MB_loadTexture(texPath);
   printf("Loaded mesh %s with texture %s\n", meshPath, texPath);

   // printBoneTree(model->bones, model->boneCount);
   // printAnimations(model->animations, model->animationCount, model->boneCount);

   return model;
}

void MB_destroy(Model * model) {
   for (int i = 0; i < model->boneCount; i++)
      free(model->bones[i].childIndices);
   free(model->bones);

   for (int i = 0; i < model->animationCount; i++) {
      for (int j = 0; j < model->boneCount; j++)
         free(model->animations[i].animBones[j].keys);
      free(model->animations[i].animBones);
   }
   free(model->animations);

   free(model);
}

