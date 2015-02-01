
#include <stdio.h>

#include "bone_controller.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

static void computeBoneTransform(glm::mat4 * transforms, glm::quat * rotations,
                                 Bone * tree, int boneIndex, glm::mat4 parentM) {
   Bone * bone = & tree[boneIndex];

   glm::mat4 rotationM = glm::toMat4(rotations[boneIndex]);
   glm::mat4 offsetM = bone->parentOffset;
   glm::mat4 bonePoseM = bone->invBonePose;

   glm::mat4 animM = parentM * offsetM * rotationM;
   transforms[boneIndex] = animM * bonePoseM;

   for (int i = 0; i < bone->childCount; i++)
      computeBoneTransform(transforms, rotations, tree, bone->childIndices[i], animM);
}

BoneController::BoneController(Model * model, glm::mat4 * boneTransforms, glm::quat * boneRotations) {
   this->model = model;
   this->boneTransforms = boneTransforms;
   this->boneRotations = boneRotations;
}

BoneController::~BoneController() {}

void BoneController::rotateBone(int boneIndex, float angle, glm::vec3 axis) {
   glm::quat rotQuat = glm::angleAxis(angle, axis);
   boneRotations[boneIndex] = boneRotations[boneIndex] * rotQuat;

   computeBoneTransform(boneTransforms, boneRotations, model->bones,
                        model->boneRoot, glm::mat4(1.0f));
}