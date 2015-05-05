
#include "terrain.h"
#include "stdio.h"

using namespace Eigen;

TerrainGenerator::TerrainGenerator() {}
TerrainGenerator::~TerrainGenerator() {}

static double randRange(float low, float high) {
    return (high - low) * rand() / (float)RAND_MAX - low;
}

static int rcToIndex(int width, int row, int col) {
   return width * row + col;
}

// Returns the normalized cross product with the up vector (v x up)
// Returns vec3(0,0,1) if v == up
static Vector3f crossWithUp(Vector3f v) {
   Vector3f r = v.cross(Vector3f(0,1,0));
   return (r(0) || r(1) || r(2)) ? r.normalized() : Vector3f(0,0,1);
}

Model * TerrainGenerator::GenerateModel() {
   // The general direction the mesh is heading towards
   Vector3f ultimateDirection = Vector3f(1.0,1.0,0.15).normalized();

   Vector3f bitangent = ultimateDirection;
   Vector3f tangent = crossWithUp(bitangent);
   Vector3f normal = tangent.cross(bitangent).normalized();

   // Set up the beginning triangle
   Vertex vTop;
   vTop.normal = normal;
   vTop.tangent = tangent;
   vTop.bitangent = bitangent;
   vTop.position = 0.5f * bitangent;
   vTop.uv = Vector2f(0.5f, 1.0f);

   Vertex vLeft;
   vLeft.normal = normal;
   vLeft.tangent = tangent;
   vLeft.bitangent = bitangent;
   vLeft.position = -0.5f * bitangent - 0.5f * tangent;
   vLeft.uv = Vector2f(0.0f, 0.0f);

   Vertex vRight;
   vRight.normal = normal;
   vRight.tangent = tangent;
   vRight.bitangent = bitangent;
   vRight.position = -0.5f * bitangent + 0.5f * tangent;
   vRight.uv = Vector2f(1.0f, 0.0f);

   Face face;
   face.vertIndices[0] = 0;
   face.vertIndices[1] = 1;
   face.vertIndices[2] = 2;
   face.normal = normal;

   // Initialize the model
   model = new Model();
   model->vertices = std::vector<Vertex>(0);
   model->faces = std::vector<Face>(0);
   model->vertices.push_back(vTop);
   model->vertices.push_back(vLeft);
   model->vertices.push_back(vRight);
   model->faces.push_back(face);
   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;
   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   // Send the triangle data to the shader
   model->bufferVertices();
   model->bufferIndices();

   // // Create the first 6 paths
   // Path p;
   // p.head = & model->vertices[0];
   // p.tail =

   return model;
}

void TerrainGenerator::BuildStep() {
   printf("--Stepped--\n");
}





