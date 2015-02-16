#include <OpenGL/gl.h>
#include <math.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

#define HFOV   M_PI/4.0
#define ASPECT 4.0 / 3.0
#define NEAR   0.1
#define FAR    1000.0

ForwardShader::ForwardShader() {
   program = SB_buildFromPaths("shaders/forward.vert.glsl", "shaders/forward.frag.glsl");
   setupHandles();
   projectionMatrix = glm::perspective(HFOV, ASPECT, NEAR, FAR);
};

ForwardShader::~ForwardShader() {};

void ForwardShader::setupHandles() {
   h_uHasNormals = glGetUniformLocation(program, "uHasNormals");
   h_uHasColors = glGetUniformLocation(program, "uHasColors");
   h_uHasTextures = glGetUniformLocation(program, "uHasTextures");
   h_uHasTansAndBitans = glGetUniformLocation(program, "uHasTansAndBitans");
   h_uHasAnimations = glGetUniformLocation(program, "uHasAnimations");

   h_uModelMatrix = glGetUniformLocation(program, "uModelMatrix");
   h_uProjViewMatrix = glGetUniformLocation(program, "uProjViewMatrix");
   h_uCameraPosition = glGetUniformLocation(program, "uCameraPosition");
   h_uLights = glGetUniformLocation(program, "uLights");
   h_uTexture = glGetUniformLocation(program, "uTexture");
   h_uBoneMatrices = glGetUniformLocation(program, "uBoneMatrices");

   h_aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
   h_aVertexNormal = glGetAttribLocation(program, "aVertexNormal");
   h_aVertexColor = glGetAttribLocation(program, "aVertexColor");
   h_aVertexUV = glGetAttribLocation(program, "aVertexUV");

   h_aBoneIndices0 = glGetAttribLocation(program, "aVertexBoneIndices0");
   h_aBoneIndices1 = glGetAttribLocation(program, "aVertexBoneIndices1");
   h_aBoneIndices2 = glGetAttribLocation(program, "aVertexBoneIndices2");
   h_aBoneIndices3 = glGetAttribLocation(program, "aVertexBoneIndices3");
   h_aBoneWeights0 = glGetAttribLocation(program, "aVertexBoneWeights0");
   h_aBoneWeights1 = glGetAttribLocation(program, "aVertexBoneWeights1");
   h_aBoneWeights2 = glGetAttribLocation(program, "aVertexBoneWeights2");
   h_aBoneWeights3 = glGetAttribLocation(program, "aVertexBoneWeights3");
   h_aNumInfluences = glGetAttribLocation(program, "aNumInfluences");
}

void ForwardShader::sendWorldData(World * world) {
   glUseProgram(program);

   glUniform3fv(h_uLights, 4*world->numLights, (GLfloat*)(world->lights));
}

void ForwardShader::sendCameraData(Camera * camera) {
   glUseProgram(program);

   glm::vec3 target = camera->position + camera->direction;
   glm::mat4 viewMatrix = glm::lookAt(camera->position, target, camera->up);
   glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
   glUniformMatrix4fv(h_uProjViewMatrix, 1, GL_FALSE, glm::value_ptr(pvMatrix));

   glUniform3fv(h_uCameraPosition, 1, glm::value_ptr(camera->position));
}

void ForwardShader::sendModelData(Model * model) {
   glUseProgram(program);

   glUniform1i(h_uHasNormals, model->hasNormals);
   glUniform1i(h_uHasColors, model->hasColors);
   glUniform1i(h_uHasTextures, model->hasTexCoords && model->hasTextures);
   glUniform1i(h_uHasTansAndBitans, model->hasTansAndBitans);
   glUniform1i(h_uHasAnimations, model->hasAnimations);

   sendVertexAttribArray(h_aVertexPosition, model->posID, 3);
   if (model->hasNormals)
      sendVertexAttribArray(h_aVertexNormal, model->normID, 3);
   if (model->hasColors)
      sendVertexAttribArray(h_aVertexColor, model->colorID, 3);
   if (model->hasTexCoords)
      sendVertexAttribArray(h_aVertexUV, model->uvID, 2);
   if (model->hasTextures)
      sendTexture(h_uTexture, model->texID, GL_TEXTURE0);
   if (model->hasBoneWeights && model->hasAnimations) {
      sendLargeVertexAttribArray(h_aBoneIndices0, h_aBoneIndices1,
                                 h_aBoneIndices2, h_aBoneIndices3,
                                 model->bIndID, model->maxInfluences);
      sendLargeVertexAttribArray(h_aBoneWeights0, h_aBoneWeights1,
                                 h_aBoneWeights2, h_aBoneWeights3,
                                 model->bWeightID, model->maxInfluences);
      sendVertexAttribArray(h_aNumInfluences, model->bNumInfID, 1);
   }

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indID);
   indexCount = 3 * model->faceCount;
}

void ForwardShader::renderEntity(Entity * entity) {
   glUseProgram(program);

   glm::mat4 transM = glm::translate(glm::mat4(1.0f), entity->position);
   glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), entity->rotation, glm::vec3(0,1,0));
   glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), entity->scale);
   glm::mat4 modelM = transM * rotateM * scaleM;
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(modelM));

   if (entity->model->hasBoneWeights && entity->model->hasAnimations)
      glUniformMatrix4fv(h_uBoneMatrices, MAX_BONES, GL_FALSE, (GLfloat *)(entity->boneTransforms));

   glDrawElements(GL_TRIANGLES, 3 * entity->model->faceCount, GL_UNSIGNED_INT, 0);
   checkOpenGLError();
}

