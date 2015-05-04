#include "shader.h"
#include "shader_builder.h"
#include "safe_gl.h"

TextureShader::TextureShader() {
   program = SB_buildFromPaths("shaders/texture.vert.glsl", "shaders/texture.frag.glsl");

   h_uProjViewModelM = glGetUniformLocation(program, "uProjViewModelM");
   h_uTexture = glGetUniformLocation(program, "uTexture");
   h_aPosition = glGetAttribLocation(program, "aPosition");
   h_aUV = glGetAttribLocation(program, "aUV");
}

TextureShader::~TextureShader() {}

void TextureShader::render(Camera * camera, LightData * lightdata, Entity * entity) {
   Model * model = entity->model;

   glUseProgram(program);

   // Send Projection, View, and Model matrices
   Eigen::Matrix4f projViewModelM = camera->getProjectionM() * camera->getViewM() * entity->generateModelM();
   glUniformMatrix4fv(h_uProjViewModelM, 1, GL_FALSE, projViewModelM.data());

   // Send vertex attributes
   sendVertexAttribArray(h_aPosition, model->vertexID, 3, offsetof(Vertex, position));
   sendVertexAttribArray(h_aUV, model->vertexID, 2, offsetof(Vertex, uv));

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