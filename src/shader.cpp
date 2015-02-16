
#include <OpenGL/gl.h>

#include "shader.h"

void EntityShader::sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size, int type) {
   glEnableVertexAttribArray(handle);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glVertexAttribPointer(handle, size, type, GL_FALSE, 0, 0);
}

void EntityShader::sendTexture(unsigned int handle, unsigned int id, GLenum texture) {
   glActiveTexture(texture);
   glBindTexture(GL_TEXTURE_2D, id);
   glUniform1i(handle, 0);
}