
#include "terrain.h"
#include "stdio.h"

TerrainGenerator::TerrainGenerator() {}
TerrainGenerator::~TerrainGenerator() {}

static double randRange(float low, float high) {
    return (high - low) * rand() / (float)RAND_MAX - low;
}

static int rcToIndex(int width, int row, int col) {
   return width * row + col;
}

Model * TerrainGenerator::GenerateModel() {
   model = new Model();

   model->vertices = std::vector<Vertex>(0);
   model->faces = std::vector<Face>(0);

   int width = 100;
   int height = 100;

   // Generate vertices
   for (int c = 0; c < width; c++) {
      for (int r = 0; r < height; r++) {
         Vertex v;
         v.position = Eigen::Vector3f(c-width/2, r, -25 + c-width/2 + randRange(-0.25, 0.25));
         v.uv = Eigen::Vector2f(c/10.0f, r/10.0f);
         model->vertices.push_back(v);
      }
   }

   // Generate faces
   for (int c = 0; c < (width-1); c++) {
      for (int r = 0; r < (height-1); r++) {
         int indexTL = rcToIndex(width, r, c);
         int indexTR = rcToIndex(width, r, c+1);
         int indexBL = rcToIndex(width, r+1, c);
         int indexBR = rcToIndex(width, r+1, c+1);

         Face face;
         face.vertIndices[0] = indexBL;
         face.vertIndices[1] = indexTR;
         face.vertIndices[2] = indexTL;
         model->faces.push_back(face);
         face.vertIndices[0] = indexBL;
         face.vertIndices[1] = indexBR;
         face.vertIndices[2] = indexTR;
         model->faces.push_back(face);
      }
   }

   // Generate normals
   model->CalculateNormals();

   // Generate tangents and bitangents
   for (int c = 0; c < width; c++) {
      for (int r = 0; r < height; r++) {
         Vertex * v = & model->vertices[rcToIndex(width, r, c)];

         v->tangent = Eigen::Vector3f(0,0,0);
         if (c > 0)
            v->tangent += (v->position - model->vertices[rcToIndex(width, r, c-1)].position).normalized();
         if (c < width-1)
            v->tangent += (model->vertices[rcToIndex(width, r, c+1)].position - v->position).normalized();
         v->tangent.normalize();

         v->bitangent = (v->normal.cross(v->tangent)).normalized();
      }
   }

   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   model->boneCount = 0;
   model->animationCount = 0;

   model->bufferVertices();
   model->bufferIndices();

   return model;
}

void TerrainGenerator::BuildStep() {
   printf("--Stepped--\n");
}





