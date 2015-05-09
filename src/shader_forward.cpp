
#include <math.h>
#include <stdio.h>

#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

ForwardShader::ForwardShader() {
   statProg = SB_buildFromPaths("shaders/forward_static.vert.glsl", "shaders/forward.frag.glsl");
   animProg = SB_buildFromPaths("shaders/forward_animated.vert.glsl", "shaders/forward.frag.glsl");
   fillHandleTable(& animTable, animProg, true);
   fillHandleTable(& statTable, statProg, false);
}

ForwardShader::~ForwardShader() {}

void ForwardShader::fillHandleTable(HandleTable * table, unsigned int program, bool animated) {
   table->uHasNormals = glGetUniformLocation(program, "uHasNormals");
   table->uHasColors = glGetUniformLocation(program, "uHasColors");
   table->uHasTexture = glGetUniformLocation(program, "uHasTexture");
   table->uHasNormalMap = glGetUniformLocation(program, "uHasNormalMap");
   table->uHasSpecularMap = glGetUniformLocation(program, "uHasSpecularMap");

   table->uModelM = glGetUniformLocation(program, "uModelM");
   table->uProjViewM = glGetUniformLocation(program, "uProjViewM");
   table->uCameraPosition = glGetUniformLocation(program, "uCameraPosition");
   table->uLights = glGetUniformLocation(program, "uLights");
   table->uTexture = glGetUniformLocation(program, "uTexture");
   table->uNormalMap = glGetUniformLocation(program, "uNormalMap");
   table->uSpecularMap = glGetUniformLocation(program, "uSpecularMap");

   table->aPosition = glGetAttribLocation(program, "aPosition");
   table->aNormal = glGetAttribLocation(program, "aNormal");
   table->aColor = glGetAttribLocation(program, "aColor");
   table->aUV = glGetAttribLocation(program, "aUV");
   table->aTangent = glGetAttribLocation(program, "aTangent");
   table->aBitangent = glGetAttribLocation(program, "aBitangent");

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
      table->uAnimMs = glGetUniformLocation(program, "uAnimMs");
   }
}

void ForwardShader::render(Camera * camera, LightData * lightData, Entity * entityBase) {

   Model * model = entityBase->model;

   unsigned int program = model->isAnimated && model->hasAnimations ? animProg : statProg;
   HandleTable * table = model->isAnimated && model->hasAnimations ? & animTable : & statTable;

   glUseProgram(program);

   // Send Projection, View, and Model matrices
   Eigen::Matrix4f projViewM = camera->getProjectionM() * camera->getViewM();
   glUniformMatrix4fv(table->uProjViewM, 1, GL_FALSE, projViewM.data());
   glUniformMatrix4fv(table->uModelM, 1, GL_FALSE, entityBase->generateModelM().data());

   // Send camera and light data
   glUniform3fv(table->uCameraPosition, 1, camera->position.data());
   glUniform3fv(table->uLights, 4*lightData->numLights, (GLfloat*)(lightData->lights));

   // Send model present flags
   glUniform1i(table->uHasNormals, model->hasNormals);
   glUniform1i(table->uHasColors, model->hasColors);
   glUniform1i(table->uHasTexture, model->hasTexCoords && model->hasTexture);
   glUniform1i(table->uHasNormalMap, model->hasTexCoords && model->hasNormalMap);
   glUniform1i(table->uHasSpecularMap, model->hasTexCoords && model->hasSpecularMap);

   // Send model attributes
   sendVertexAttribArray(table->aPosition, model->posID, 3);

   if (model->hasNormals)
      sendVertexAttribArray(table->aNormal, model->normID, 3);
   if (model->hasColors)
      sendVertexAttribArray(table->aColor, model->colorID, 3);
   if (model->hasTexCoords) {
      sendVertexAttribArray(table->aUV, model->uvID, 2);

      if (model->hasTansAndBitans) {
         sendVertexAttribArray(table->aTangent, model->tanID, 3);
         sendVertexAttribArray(table->aBitangent, model->bitanID, 3);
      }

      if (model->hasTexture)
         sendTexture(table->uTexture, model->texID, GL_TEXTURE0);
      if (model->hasNormalMap)
         sendTexture(table->uNormalMap, model->nmapID, GL_TEXTURE1);
      if (model->hasSpecularMap)
         sendTexture(table->uSpecularMap, model->smapID, GL_TEXTURE2);
   }

   // Send animation data
   if (model->isAnimated) {
      sendVertexAttribArray(table->aNumInfluences, model->bNumInfID, 1);
      sendLargeVertexAttribArray(table->aBoneIndices0, table->aBoneIndices1,
                                 table->aBoneIndices2, table->aBoneIndices3,
                                 model->bIndexID);
      sendLargeVertexAttribArray(table->aBoneWeights0, table->aBoneWeights1,
                                 table->aBoneWeights2, table->aBoneWeights3,
                                 model->bWeightID);
      AnimatedEntity * entityAnim = dynamic_cast<AnimatedEntity *>(entityBase);
      glUniformMatrix4fv(table->uAnimMs, MAX_BONES, GL_FALSE, (GLfloat *)(entityAnim->animMs));
   }

   // Draw the damn thing!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexID);
   glDrawElements(GL_TRIANGLES, 3 * model->faceCount, GL_UNSIGNED_INT, 0);

   // cleanup
   glUseProgram(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDisableVertexAttribArray(table->aPosition);
   glDisableVertexAttribArray(table->aNormal);
   glDisableVertexAttribArray(table->aColor);
   glDisableVertexAttribArray(table->aUV);
   glDisableVertexAttribArray(table->aTangent);
   glDisableVertexAttribArray(table->aBitangent);

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
