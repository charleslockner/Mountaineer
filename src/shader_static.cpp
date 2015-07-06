
/*
 * Mountaineer - A Rock Climbing Engine
 * Charles Lockner
 * Ask me before using this, or you shall be judged. Copyright 2015
 *
 * shader_static.cpp
 * Shades an entity without displaying its animation, even if it has one.
 */

#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

StaticShader::StaticShader() {
   program = SB::BuildProgramFromPaths("shaders/forward_static.vert.glsl", "shaders/forward.frag.glsl");

   h_uHasNormals        = glGetUniformLocation(program, "uHasNormals");
   h_uHasColors         = glGetUniformLocation(program, "uHasColors");
   h_uHasTexture        = glGetUniformLocation(program, "uHasTexture");
   h_uHasNormalMap      = glGetUniformLocation(program, "uHasNormalMap");
   h_uHasSpecularMap    = glGetUniformLocation(program, "uHasSpecularMap");

   h_uModelM            = glGetUniformLocation(program, "uModelM");
   h_uProjViewM         = glGetUniformLocation(program, "uProjViewM");
   h_uCameraPosition    = glGetUniformLocation(program, "uCameraPosition");
   h_uLights            = glGetUniformLocation(program, "uLights");
   h_uTexture           = glGetUniformLocation(program, "uTexture");
   h_uNormalMap         = glGetUniformLocation(program, "uNormalMap");
   h_uSpecularMap       = glGetUniformLocation(program, "uSpecularMap");

   h_aPosition          = glGetAttribLocation(program, "aPosition");
   h_aNormal            = glGetAttribLocation(program, "aNormal");
   h_aColor             = glGetAttribLocation(program, "aColor");
   h_aTangent           = glGetAttribLocation(program, "aTangent");
   h_aBitangent         = glGetAttribLocation(program, "aBitangent");
   h_aUV                = glGetAttribLocation(program, "aUV");
}

StaticShader::~StaticShader() {}

void StaticShader::render(Camera * camera, LightData * lightData, StaticEntity * entity) {
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

   checkOpenGLError();
}