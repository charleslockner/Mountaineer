
#include <iostream>
#include <string>
#include <vector>

#include "tiny_obj_loader.h"
#include "safe_gl.h"
#include "matrix_math.h"
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
   this->faceCount = indBuf.size() / NUM_FACE_EDGES;
   this->boneCount = 0;
   this->animationCount = 0;

   this->vertices = std::vector<Vertex>(this->vertexCount);
   this->faces = std::vector<Face>(this->faceCount);

   for (int i = 0; i < this->vertexCount; i++)
      this->vertices[i].position = Eigen::Vector3f(posBuf[3*i], posBuf[3*i+1], posBuf[3*i+2]);

   if (!norBuf.empty())
      for (int i = 0; i < this->vertexCount; i++)
         this->vertices[i].normal = Eigen::Vector3f(norBuf[3*i], norBuf[3*i+1], norBuf[3*i+2]);

   if (!texBuf.empty())
      for (int i = 0; i < this->vertexCount; i++)
         this->vertices[i].uv = Eigen::Vector2f(texBuf[2*i], texBuf[2*i+1]);

   for (int i = 0; i < this->faceCount; i++)
      for (int j = 0; j < NUM_FACE_EDGES; j++)
         this->faces[i].vertIndices[j] = indBuf[3*i+j];

   // Send vertex and face data to the GPU
   bufferVertices();
   bufferIndices();

   checkOpenGLError();

   std::cerr << "Loaded OBJ model: " << path << "\n";
}
