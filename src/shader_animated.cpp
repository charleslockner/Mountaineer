
/*
 * Mountaineer - A Rock Climbing Engine
 * Charles Lockner
 * Ask me before using this, or you shall be judged. Copyright 2015
 *
 * shader_animated.cpp
 * Renders an animated entity accorording to the bone matrices, and bone weights/indices.
 */

#include <math.h>
#include <stdio.h>

#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

AnimatedShader::AnimatedShader() {
   program = SB::BuildProgramFromPaths("shaders/forward_animated.vert.glsl", "shaders/forward.frag.glsl");

   h_uHasNormals     = glGetUniformLocation(program, "uHasNormals");
   h_uHasColors      = glGetUniformLocation(program, "uHasColors");
   h_uHasTexture     = glGetUniformLocation(program, "uHasTexture");
   h_uHasNormalMap   = glGetUniformLocation(program, "uHasNormalMap");
   h_uHasSpecularMap = glGetUniformLocation(program, "uHasSpecularMap");

   h_uModelM         = glGetUniformLocation(program, "uModelM");
   h_uProjViewM      = glGetUniformLocation(program, "uProjViewM");
   h_uCameraPosition = glGetUniformLocation(program, "uCameraPosition");
   h_uLights         = glGetUniformLocation(program, "uLights");
   h_uTexture        = glGetUniformLocation(program, "uTexture");
   h_uNormalMap      = glGetUniformLocation(program, "uNormalMap");
   h_uSpecularMap    = glGetUniformLocation(program, "uSpecularMap");

   h_uAnimMs         = glGetUniformLocation(program, "uAnimMs");

   h_aPosition       = glGetAttribLocation(program, "aPosition");
   h_aNormal         = glGetAttribLocation(program, "aNormal");
   h_aColor          = glGetAttribLocation(program, "aColor");
   h_aUV             = glGetAttribLocation(program, "aUV");
   h_aTangent        = glGetAttribLocation(program, "aTangent");
   h_aBitangent      = glGetAttribLocation(program, "aBitangent");

   h_aBoneIndices0   = glGetAttribLocation(program, "aBoneIndices0");
   h_aBoneIndices1   = glGetAttribLocation(program, "aBoneIndices1");
   h_aBoneIndices2   = glGetAttribLocation(program, "aBoneIndices2");
   h_aBoneIndices3   = glGetAttribLocation(program, "aBoneIndices3");
   h_aBoneWeights0   = glGetAttribLocation(program, "aBoneWeights0");
   h_aBoneWeights1   = glGetAttribLocation(program, "aBoneWeights1");
   h_aBoneWeights2   = glGetAttribLocation(program, "aBoneWeights2");
   h_aBoneWeights3   = glGetAttribLocation(program, "aBoneWeights3");
   h_aNumInfluences  = glGetAttribLocation(program, "aNumInfluences");
}

AnimatedShader::~AnimatedShader() {}


void AnimatedShader::render(Camera * camera, LightData * lightData, AnimatedEntity * entity) {
   Model * model = entity->model;

   glUseProgram(program);

   // Send Projection, View, and Model matrices
   Eigen::Matrix4f projViewM = camera->getProjectionM() * camera->getViewM();
   glUniformMatrix4fv(h_uProjViewM, 1, GL_FALSE, projViewM.data());
   glUniformMatrix4fv(h_uModelM, 1, GL_FALSE, entity->generateModelM().data());

   // Send camera and light data
   glUniform3fv(h_uCameraPosition, 1, camera->position.data());
   glUniform3fv(h_uLights, 4*lightData->numLights, (GLfloat*)(lightData->lights));

   // Send model present flags
   glUniform1i(h_uHasNormals, model->hasNormals);
   glUniform1i(h_uHasColors, model->hasColors);
   glUniform1i(h_uHasTexture, model->hasTexCoords && model->hasTexture);
   glUniform1i(h_uHasNormalMap, model->hasTexCoords && model->hasNormalMap);
   glUniform1i(h_uHasSpecularMap, model->hasTexCoords && model->hasSpecularMap);

   // Send model attributes
   sendVertexAttribArray(h_aPosition, model->posID, 3);

   if (model->hasNormals)
      sendVertexAttribArray(h_aNormal, model->normID, 3);
   if (model->hasColors)
      sendVertexAttribArray(h_aColor, model->colorID, 3);
   if (model->hasTexCoords) {
      sendVertexAttribArray(h_aUV, model->uvID, 2);

      if (model->hasTansAndBitans) {
         sendVertexAttribArray(h_aTangent, model->tanID, 3);
         sendVertexAttribArray(h_aBitangent, model->bitanID, 3);
      }

      if (model->hasTexture)
         sendTexture(h_uTexture, model->texID, GL_TEXTURE0);
      if (model->hasNormalMap)
         sendTexture(h_uNormalMap, model->nmapID, GL_TEXTURE1);
      if (model->hasSpecularMap)
         sendTexture(h_uSpecularMap, model->smapID, GL_TEXTURE2);
   }

   // Send animation data
   sendVertexAttribArray(h_aNumInfluences, model->bNumInfID, 1);
   sendLargeVertexAttribArray(h_aBoneIndices0, h_aBoneIndices1,
                              h_aBoneIndices2, h_aBoneIndices3,
                              model->bIndexID);
   sendLargeVertexAttribArray(h_aBoneWeights0, h_aBoneWeights1,
                              h_aBoneWeights2, h_aBoneWeights3,
                              model->bWeightID);
   glUniformMatrix4fv(h_uAnimMs, MAX_BONES, GL_FALSE, (GLfloat *)(entity->animMs));

   // Draw the damn thing!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexID);
   glDrawElements(GL_TRIANGLES, 3 * model->faceCount, GL_UNSIGNED_INT, 0);

   // cleanup
   glUseProgram(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDisableVertexAttribArray(h_aPosition);
   glDisableVertexAttribArray(h_aNormal);
   glDisableVertexAttribArray(h_aColor);
   glDisableVertexAttribArray(h_aUV);
   glDisableVertexAttribArray(h_aTangent);
   glDisableVertexAttribArray(h_aBitangent);

   glDisableVertexAttribArray(h_aBoneIndices0);
   glDisableVertexAttribArray(h_aBoneIndices1);
   glDisableVertexAttribArray(h_aBoneIndices2);
   glDisableVertexAttribArray(h_aBoneIndices3);
   glDisableVertexAttribArray(h_aBoneWeights0);
   glDisableVertexAttribArray(h_aBoneWeights1);
   glDisableVertexAttribArray(h_aBoneWeights2);
   glDisableVertexAttribArray(h_aBoneWeights3);
   glDisableVertexAttribArray(h_aNumInfluences);

   checkOpenGLError();
}
