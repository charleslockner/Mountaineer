#include "model.h"
#include "stdio.h"
#include "safe_gl.h"

#include <cstring>

// ======================================================== //
// ===================== FACE METHODS ===================== //
// ======================================================== //

void Face::calculateNormal() {
   Vertex * v1 = vertices[0];
   Vertex * v2 = vertices[1];
   Vertex * v3 = vertices[2];
   normal = ((v3->position - v2->position).cross(v1->position - v2->position)).normalized();
}

Eigen::Vector3f Face::intersectRay(Geom::Rayf ray) {
   calculateNormal();
   Geom::Planef plane = Geom::Planef(vertices[0]->position, normal);
   return Geom::Intersectf(ray, plane);
}

bool Face::pointCheckInside(Eigen::Vector3f pnt) {
   Vertex * v1 = vertices[0];
   Vertex * v2 = vertices[1];
   Vertex * v3 = vertices[2];

   bool inside12 = (v2->position - v1->position).cross(pnt - v1->position).dot(normal) >= 0;
   bool inside23 = (v3->position - v2->position).cross(pnt - v2->position).dot(normal) >= 0;
   bool inside31 = (v1->position - v3->position).cross(pnt - v3->position).dot(normal) >= 0;

   return inside12 && inside23 && inside31;
}

// ======================================================== //
// ==================== MODEL METHODS ===================== //
// ======================================================== //

Model::Model() {
   hasNormals = false;
   hasColors = false;
   hasTexCoords = false;
   hasTexture = false;
   hasNormalMap = false;
   hasSpecularMap = false;
   hasTansAndBitans = false;
   hasBoneWeights = false;
   hasBoneTree = false;
   hasAnimations = false;
   isAnimated = false;

   vertexCount = 0;
   faceCount = 0;
   boneCount = 0;
   animationCount = 0;

   glGenBuffers(1, & posID);
   glGenBuffers(1, & normID);
   glGenBuffers(1, & colorID);
   glGenBuffers(1, & uvID);
   glGenBuffers(1, & tanID);
   glGenBuffers(1, & bitanID);
   glGenBuffers(1, & bNumInfID);
   glGenBuffers(1, & bIndexID);
   glGenBuffers(1, & bWeightID);
   glGenBuffers(1, & indexID);
}

Model::~Model() {
}

void Model::CalculateNormals() {
   // Zero out vertex normals so we can sum from face normals starting at 0
   for (int i = 0; i < vertices.size(); i++)
      vertices[i]->normal = Eigen::Vector3f(0,0,0);

   // Calculated face normals and add these to neighboring vertex normals
   for (int i = 0; i < faces.size(); i++) {
      Face * face = faces[i];

      // Fill in face normals
      face->calculateNormal();

      // Add face normal to neighboring vertex normals
      face->vertices[0]->normal += face->normal;
      face->vertices[1]->normal += face->normal;
      face->vertices[2]->normal += face->normal;
   }

   // Normalize the new vertex normals
   for (int i = 0; i < vertices.size(); i++)
      vertices[i]->normal.normalize();
}

static void bufferVertexField(char ** ptrData, size_t offset, unsigned int vbo, int size, int count) {
   int numBytes = size * count;
   char * toGPU = (char *)malloc(numBytes);

   for (int i = 0; i < count; i++)
      std::memcpy(& toGPU[size*i], & (ptrData[i][offset]), size);

   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, numBytes, toGPU, GL_DYNAMIC_DRAW);
   free(toGPU);
}

void Model::bufferVertices() {
   char ** vertPtr = (char **)(vertices.data());
   bufferVertexField(vertPtr, offsetof(Vertex, position),     posID,     3*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, normal),       normID,    3*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, color),        colorID,   3*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, uv),           uvID,      2*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, tangent),      tanID,     3*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, bitangent),    bitanID,   3*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, boneInfCount), bNumInfID, 1*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, boneIndices),  bIndexID,  4*sizeof(float), vertices.size());
   bufferVertexField(vertPtr, offsetof(Vertex, boneWeights),  bWeightID, 4*sizeof(float), vertices.size());
}

void Model::bufferIndices() {
   int numBytes = sizeof(unsigned int) * NUM_FACE_EDGES * faces.size();
   unsigned int * indices = (unsigned int *)malloc(numBytes);
   for (int i = 0; i < faces.size(); i++)
      for (int j = 0; j < NUM_FACE_EDGES; j++)
         indices[NUM_FACE_EDGES*i+j] = faces[i]->vertices[j]->index;

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexID);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, numBytes, indices, GL_STATIC_DRAW);
   free(indices);
}

void Model::printVertices() {
   for (int i = 0; i < vertexCount; i++) {
      Vertex * v = vertices[i];
      printf("Vertex %d:\n", i);
      printf("  position = %f %f %f\n", v->position(0), v->position(1), v->position(2));
      printf("  normal = %f %f %f\n", v->normal(0), v->normal(1), v->normal(2));
   }
}

void Model::printFaces() {
   for (int i = 0; i < faceCount; i++) {
      printf("Face %d:\n", i);
      printf("  vertIndices: %d %d %d\n", faces[i]->vertices[0]->index, faces[i]->vertices[1]->index, faces[i]->vertices[2]->index);
   }
}

void Model::printBoneTree() {
   for (int i = 0; i < boneCount; i++) {
      Bone * bone = & bones[i];
      printf("Bone: %d\n", i);
      printf("  parent: %d\n", bone->parentIndex);
      for (int j = 0; j < bone->childCount; j++) {
         printf("  child: %d\n", bone->childIndices[j]);
      }
      Eigen::Matrix4f m = bone->invBonePose;
      printf("  invBonePose:\n");
      printf("    %.2f %.2f %.2f %.2f\n", m(0,0), m(1,0), m(2,0), m(3,0));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,1), m(1,1), m(2,1), m(3,1));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,2), m(1,2), m(2,2), m(3,2));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,3), m(1,3), m(2,3), m(3,3));
      m = bone->parentOffset;
      printf("  parentOffset:\n");
      printf("    %.2f %.2f %.2f %.2f\n", m(0,0), m(1,0), m(2,0), m(3,0));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,1), m(1,1), m(2,1), m(3,1));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,2), m(1,2), m(2,2), m(3,2));
      printf("    %.2f %.2f %.2f %.2f\n", m(0,3), m(1,3), m(2,3), m(3,3));
   }
}

void Model::printAnimations() {
   for (int i = 0; i < animationCount; i++) {
      Animation * anim = & animations[i];

      printf("Animation %d:\n", i);
      printf("fps %d\n", anim->fps);
      printf("keyCount %d\n", anim->keyCount);

      for (int j = 0; j < boneCount; j++) {
         AnimBone * animBone = & anim->animBones[j];
         printf("  AnimBone %d:\n", j);

         for (int k = 0; k < anim->keyCount; k++) {
            Key * key = & animBone->keys[k];
            printf("    key %d at time %f:\n", k, key->time);
            printf("      trans: %.2f %.2f %.2f\n", key->position.x(), key->position.y(), key->position.z());
            printf("      rotate: %.2f %.2f %.2f %.2f\n", key->rotation.w(), key->rotation.x(), key->rotation.y(), key->rotation.z());
            printf("      scale: %.2f %.2f %.2f\n", key->scale.x(), key->scale.y(), key->scale.z());
         }
      }
   }
}
