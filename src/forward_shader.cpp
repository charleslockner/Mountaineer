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

ForwardShader::ForwardShader() {
   statProg = SB_buildFromPaths("shaders/forward_static.vert.glsl", "shaders/forward.frag.glsl");
   animProg = SB_buildFromPaths("shaders/forward_animated.vert.glsl", "shaders/forward.frag.glsl");
   fillHandleTable(& animTable, animProg, true);
   fillHandleTable(& statTable, statProg, false);
};

ForwardShader::~ForwardShader() {};

void ForwardShader::fillHandleTable(HandleTable * table, unsigned int program, bool animated) {
   table->uHasNormals = glGetUniformLocation(program, "uHasNormals");
   table->uHasColors = glGetUniformLocation(program, "uHasColors");
   table->uHasTextures = glGetUniformLocation(program, "uHasTextures");
   table->uHasTansAndBitans = glGetUniformLocation(program, "uHasTansAndBitans");

   table->uModelM = glGetUniformLocation(program, "uModelM");
   table->uProjViewM = glGetUniformLocation(program, "uProjViewM");
   table->uCameraPosition = glGetUniformLocation(program, "uCameraPosition");
   table->uLights = glGetUniformLocation(program, "uLights");
   table->uTexture = glGetUniformLocation(program, "uTexture");

   table->aPosition = glGetAttribLocation(program, "aPosition");
   table->aNormal = glGetAttribLocation(program, "aNormal");
   table->aColor = glGetAttribLocation(program, "aColor");
   table->aUV = glGetAttribLocation(program, "aUV");

   if (animated) {
      table->aBoneIndices0 = glGetAttribLocation(program, "aBoneIndices0");
      table->aBoneIndices1 = glGetAttribLocation(program, "aBoneIndices1");
      table->aBoneIndices2 = glGetAttribLocation(program, "aBoneIndices2");
      table->aBoneIndices3 = glGetAttribLocation(program, "aBoneIndices3");
      table->aBoneWeights0 = glGetAttribLocation(program, "aBoneWeights0");
      table->aBoneWeights1 = glGetAttribLocation(program, "aBoneWeights1");
      table->aBoneWeights2 = glGetAttribLocation(program, "aBoneWeights2");
      table->aBoneWeights3 = glGetAttribLocation(program, "aBoneWeights3");
      table->aNumInfluences = glGetAttribLocation(program, "aNumInfluences");
      table->uBoneMs = glGetUniformLocation(program, "uBoneMs");
   }
}

void ForwardShader::render(Camera * camera, LightData * lightData, Entity * entity) {
   Model * model = entity->model;

   unsigned int program = model->isAnimated ? animProg : statProg;
   HandleTable * table = model->isAnimated ? & animTable : & statTable;

   glUseProgram(program);

   // Send Projection, View, and Model matrices
   glUniformMatrix4fv(table->uProjViewM, 1, GL_FALSE, glm::value_ptr(camera->generateProjViewM()));
   glUniformMatrix4fv(table->uModelM, 1, GL_FALSE, glm::value_ptr(entity->generateModelM()));

   // Send camera and light data
   glUniform3fv(table->uCameraPosition, 1, glm::value_ptr(camera->position));
   glUniform3fv(table->uLights, 4*lightData->numLights, (GLfloat*)(lightData->lights));

   // Send model present flags
   glUniform1i(table->uHasNormals, model->hasNormals);
   glUniform1i(table->uHasColors, model->hasColors);
   glUniform1i(table->uHasTextures, model->hasTexCoords && model->hasTextures);
   glUniform1i(table->uHasTansAndBitans, model->hasTansAndBitans);

   // Send model attributes
   sendVertexAttribArray(table->aPosition, model->posID, 3);
   if (model->hasNormals) sendVertexAttribArray(table->aNormal, model->normID, 3);
   if (model->hasColors) sendVertexAttribArray(table->aColor, model->colorID, 3);
   if (model->hasTexCoords) sendVertexAttribArray(table->aUV, model->uvID, 2);
   if (model->hasTextures) sendTexture(table->uTexture, model->texID, GL_TEXTURE0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indID);

   // Send animation data
   if (model->isAnimated) {
      sendLargeVertexAttribArray(table->aBoneIndices0, table->aBoneIndices1,
                                 table->aBoneIndices2, table->aBoneIndices3,
                                 model->bIndID, model->maxInfluences);
      sendLargeVertexAttribArray(table->aBoneWeights0, table->aBoneWeights1,
                                 table->aBoneWeights2, table->aBoneWeights3,
                                 model->bWeightID, model->maxInfluences);
      sendVertexAttribArray(table->aNumInfluences, model->bNumInfID, 1);
      glUniformMatrix4fv(table->uBoneMs, MAX_BONES, GL_FALSE, (GLfloat *)(entity->boneTransforms));
   }

   // Draw the damn thing!
   glDrawElements(GL_TRIANGLES, 3 * model->faceCount, GL_UNSIGNED_INT, 0);

   // cleanup
   glUseProgram(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDisableVertexAttribArray(table->aPosition);
   glDisableVertexAttribArray(table->aNormal);
   glDisableVertexAttribArray(table->aColor);
   glDisableVertexAttribArray(table->aUV);
   if (model->isAnimated) {
      glDisableVertexAttribArray(table->aBoneIndices0);
      glDisableVertexAttribArray(table->aBoneIndices1);
      glDisableVertexAttribArray(table->aBoneIndices2);
      glDisableVertexAttribArray(table->aBoneIndices3);
      glDisableVertexAttribArray(table->aBoneWeights0);
      glDisableVertexAttribArray(table->aBoneWeights1);
      glDisableVertexAttribArray(table->aBoneWeights2);
      glDisableVertexAttribArray(table->aBoneWeights3);
      glDisableVertexAttribArray(table->aNumInfluences);
   }

   checkOpenGLError();
}


