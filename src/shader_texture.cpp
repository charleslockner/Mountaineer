
/*
 * Mountaineer - A Rock Climbing Engine
 * Charles Lockner
 * Ask me before using this, or you shall be judged. Copyright 2015
 *
 * shader_texture.cpp
 * Shades an entity with a texture. Doesn't care about lighting. Only useful for rendering a skybox.
 */

#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

TextureShader::TextureShader() {
   program = SB::BuildProgramFromPaths("shaders/texture.vert.glsl", "shaders/texture.frag.glsl");

   h_uProjViewModelM = glGetUniformLocation(program, "uProjViewModelM");
   h_uTexture = glGetUniformLocation(program, "uTexture");
   h_aPosition = glGetAttribLocation(program, "aPosition");
   h_aUV = glGetAttribLocation(program, "aUV");
}

TextureShader::~TextureShader() {}

void TextureShader::render(Camera * camera, LightData * lightdata, StaticEntity * entity) {
   Model * model = entity->model;

   glUseProgram(program);

   // Send Projection, View, and Model matrices
   Eigen::Matrix4f projViewModelM = camera->getProjectionM() * camera->getViewM() * entity->generateModelM();
   glUniformMatrix4fv(h_uProjViewModelM, 1, GL_FALSE, projViewModelM.data());

   // Send vertex attributes
   sendVertexAttribArray(h_aPosition, model->posID, 3);
   sendVertexAttribArray(h_aUV, model->uvID, 2);

   // Send textures
   sendTexture(h_uTexture, model->texID, GL_TEXTURE0);

   // Draw the damn thing!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexID);
   glDrawElements(GL_TRIANGLES, 3 * model->faceCount, GL_UNSIGNED_INT, 0);

   // cleanup
   glUseProgram(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDisableVertexAttribArray(h_aPosition);
   glDisableVertexAttribArray(h_aUV);

   checkOpenGLError();
}