
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "safe_gl.h"
#include "model.h"

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

static void readHeader(FILE *fp, Model * model) {
   fread(& model->vertexCount, sizeof(unsigned int), 1, fp);
   fread(& model->faceCount, sizeof(unsigned int), 1, fp);
   fread(& model->boneCount, sizeof(unsigned int), 1, fp);
   fread(& model->animationCount, sizeof(unsigned int), 1, fp);

   if (model->boneCount > MAX_BONES) {
      printf("There are %d bones and the max is %d\n", model->boneCount, MAX_BONES);
      exit(1);
   }

   model->vertices = std::vector<Vertex *>(model->vertexCount);
   for (int i = 0; i < model->vertices.size(); i++) {
      model->vertices[i] = new Vertex();
      model->vertices[i]->index = i;
   }

   model->faces = std::vector<Face *>(model->faceCount);
   for (int i = 0; i < model->faces.size(); i++)
      model->faces[i] = new Face();

   printf("verts: %d, faces: %d, bones: %d, anims: %d\n", model->vertexCount, model->faceCount, model->boneCount, model->animationCount);
}

static void readPositions(FILE *fp, Model * model) {
   int count = 3 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->position = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

   free(data);
}

static void readNormals(FILE *fp, Model * model) {
   int count = 3 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->normal = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

   free(data);
}

static void readColors(FILE *fp, Model * model) {
   int count = 3 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->color = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

   free(data);
}

static void readTexCoords(FILE *fp, Model * model) {
   int count = 2 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->uv = Eigen::Vector2f(data[2*i], data[2*i+1]);

   free(data);
}

static void readTangents(FILE *fp, Model * model) {
   int count = 3 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->tangent = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

   free(data);
}

static void readBitangents(FILE *fp, Model * model) {
   int count = 3 * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->bitangent = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

   free(data);
}

static void readIndices(FILE *fp, Model * model) {
   int count = NUM_FACE_EDGES * model->faceCount;
   int size = sizeof(unsigned int);

   unsigned int * data = (unsigned int *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->faceCount; i++)
      for (int j = 0; j < NUM_FACE_EDGES; j++)
         model->faces[i]->vertices[j] = model->vertices[data[NUM_FACE_EDGES*i+j]];

   free(data);
}

static void readBoneIndices(FILE *fp, Model * model) {
   int count = MAX_INFLUENCES * model->vertexCount;

   unsigned int * uintData = (unsigned int *)malloc(count * sizeof(unsigned int));
   fread(uintData, sizeof(unsigned int), count, fp);

   // convert the data from unsigned ints to floats
   float * floatData = (float *)malloc(count * sizeof(float));
   for (int i = 0; i < count; i++)
      floatData[i] = uintData[i];

   for (int i = 0; i < model->vertexCount; i++)
      for (int j = 0; j < MAX_INFLUENCES; j++)
         model->vertices[i]->boneIndices[j] = floatData[MAX_INFLUENCES * i + j];

   // free the data blocks
   free(uintData);
   free(floatData);
}

static void readBoneWeights(FILE *fp, Model * model) {
   int count = MAX_INFLUENCES * model->vertexCount;
   int size = sizeof(float);

   float * data = (float *)malloc(count * size);
   fread(data, size, count, fp);

   for (int i = 0; i < model->vertexCount; i++)
      for (int j = 0; j < MAX_INFLUENCES; j++)
         model->vertices[i]->boneWeights[j] = data[MAX_INFLUENCES * i + j];

   free(data);
}

static void readBoneInfluences(FILE *fp, Model * model) {
   int count = model->vertexCount;

   unsigned int * uintData = (unsigned int *)malloc(count * sizeof(unsigned int));
   fread(uintData, sizeof(unsigned int), count, fp);

   // convert the data from unsigned ints to floats
   float * floatData = (float *)malloc(count * sizeof(float));
   for (int i = 0; i < count; i++)
      floatData[i] = uintData[i];

   for (int i = 0; i < model->vertexCount; i++)
      model->vertices[i]->boneInfCount = floatData[i];

   // free the data blocks
   free(uintData);
   free(floatData);
}

static void readBoneTree(FILE *fp, Model * model) {
   fread(& model->boneRoot, sizeof(int), 1, fp);
   printf("bone root after read %d\n", model->boneRoot);

   model->bones = std::vector<Bone>(model->boneCount);

   for (int i = 0; i < model->boneCount; i++) {
      Bone * bone = & model->bones[i];
      fread(& bone->parentIndex, sizeof(int), 1, fp);
      fread(& bone->childCount, sizeof(unsigned int), 1, fp);

      bone->childIndices = std::vector<int>(bone->childCount);
      for (int j = 0; j < bone->childCount; j++)
         fread(& bone->childIndices[j], sizeof(int), 1, fp);

      fread(& bone->invBonePose, sizeof(Eigen::Matrix4f), 1, fp);
      fread(& bone->parentOffset, sizeof(Eigen::Matrix4f), 1, fp);
   }
}

static void readAnimations(FILE *fp, Model * model) {
   model->animations = std::vector<Animation>(model->animationCount);

   for (int i = 0; i < model->animationCount; i++) {
      Animation * anim = & model->animations[i];

      fread(& anim->fps, sizeof(unsigned int), 1, fp);
      fread(& anim->keyCount, sizeof(unsigned int), 1, fp);
      anim->duration = 1.0 * (anim->keyCount-1) / anim->fps;

      anim->animBones = std::vector<AnimBone>(model->boneCount);
      for (int j = 0; j < model->boneCount; j++) {
         AnimBone * animBone = & anim->animBones[j];

         animBone->keys = std::vector<Key>(anim->keyCount);
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
   model->hasBoneWeights = isFieldPresent(flags, BONE_INDICES) &&
                           isFieldPresent(flags, BONE_WEIGHTS);
   model->hasBoneTree = isFieldPresent(flags, BONE_TREE);
   model->hasAnimations = isFieldPresent(flags, ANIMATIONS);
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
         case BONE_NUM_INF:
            readBoneInfluences(fp, model);
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

   model->bufferVertices();
   model->bufferIndices();

   checkPresentFields(model, receivedFlags);
   model->isAnimated = model->hasBoneWeights && model->hasAnimations;

   checkOpenGLError();
}

static FILE * safe_fopen(const char * path) {
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

void Model::loadCIAB(const char * path) {
   FILE * fp = safe_fopen(path);
   loadMeshData(fp, this);
   fclose(fp);

   // printBoneTree();
   // printAnimations();

   fprintf(stderr, "Loaded CIAB model: %s\n", path);
}
