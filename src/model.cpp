#include "model.h"
#include "stdio.h"

Model::Model() {
   hasNormals = false;
   hasColors = false;
   hasTexCoords = false;
   hasTextures = false;
   hasTansAndBitans = false;
   hasBoneWeights = false;
   hasBoneTree = false;
   hasAnimations = false;
}

Model::~Model() {
   if (hasBoneTree)
      for (int i = 0; i < boneCount; i++)
         free(bones[i].childIndices);
   free(bones);

   for (int i = 0; i < animationCount; i++) {
      for (int j = 0; j < boneCount; j++)
         free(animations[i].animBones[j].keys);
      free(animations[i].animBones);
   }
   free(animations);
}

void Model::printBoneTree() {
   for (int i = 0; i < boneCount; i++) {
      Bone * bone = & bones[i];
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

void Model::printAnimations() {
   for (int i = 0; i < animationCount; i++) {
      Animation * anim = & animations[i];

      printf("Animation %d:\n", i);
      printf("fps %d\n", anim->fps);
      printf("keyCount %d\n", anim->keyCount);

      for (int j = 0; j < boneCount; j++) {
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