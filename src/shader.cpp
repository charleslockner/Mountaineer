
#include "shader.h"

void EntityShader::sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size) {
   glEnableVertexAttribArray(handle);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glVertexAttribPointer(handle, size, GL_FLOAT, GL_FALSE, 0, 0);
}

void EntityShader::sendLargeVertexAttribArray(unsigned int handle0, unsigned int handle1,
                                              unsigned int handle2, unsigned int handle3,
                                              unsigned int vbo, int maxInfluences) {
   unsigned stride = maxInfluences * sizeof(float);

   glEnableVertexAttribArray(handle0);
   glEnableVertexAttribArray(handle1);
   glEnableVertexAttribArray(handle2);
   glEnableVertexAttribArray(handle3);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glVertexAttribPointer(handle0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 0*sizeof(float)));
   glVertexAttribPointer(handle1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 4*sizeof(float)));
   glVertexAttribPointer(handle2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 8*sizeof(float)));
   glVertexAttribPointer(handle3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12*sizeof(float)));
}

void EntityShader::sendTexture(unsigned int handle, unsigned int id, GLenum unit) {
   glActiveTexture(unit);
   glBindTexture(GL_TEXTURE_2D, id);
   glUniform1i(handle, unit - GL_TEXTURE0);
}

