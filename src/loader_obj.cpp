
#include <iostream>
#include <string>
#include <vector>

#include "tiny_obj_loader.h"
#include "safe_gl.h"
#include "model.h"

void Model::loadOBJ(const char * path)
{
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> objMaterials;
   std::string err = tinyobj::LoadObj(shapes, objMaterials, path);
   if (!err.empty())
      std::cerr << err << std::endl;

   const std::vector<float> &posBuf = shapes[0].mesh.positions;
   const std::vector<float> &norBuf = shapes[0].mesh.normals;
   const std::vector<float> &texBuf = shapes[0].mesh.texcoords;
   const std::vector<unsigned int> &indBuf = shapes[0].mesh.indices;

   this->hasNormals = !norBuf.empty();
   this->hasTexCoords = !texBuf.empty();

   this->vertexCount = posBuf.size() / 3;
   this->faceCount = indBuf.size();
   this->boneCount = 0;
   this->animationCount = 0;

   // Send the position array to the GPU
   glGenBuffers(1, & this->posID);
   glBindBuffer(GL_ARRAY_BUFFER, this->posID);
   glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

   // Send the normal array (if it exists) to the GPU
   if(!norBuf.empty()) {
      glGenBuffers(1, & this->normID);
      glBindBuffer(GL_ARRAY_BUFFER, this->normID);
      glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
   }

   // Send the texture coordinates array (if it exists) to the GPU
   if(!texBuf.empty()) {
      glGenBuffers(1, & this->uvID);
      glBindBuffer(GL_ARRAY_BUFFER, this->uvID);
      glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
   }

   // Send the index array to the GPU
   glGenBuffers(1, & this->indID);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indID);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

   // Unbind the arrays
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   checkOpenGLError();
}
