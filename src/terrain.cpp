
#include "terrain.h"
#include "stdio.h"

using namespace Eigen;

TerrainGenerator::TerrainGenerator() {}
TerrainGenerator::~TerrainGenerator() {}

static double randRange(float low, float high) {
   return (high - low) * rand() / (float)RAND_MAX + low;
}

static Vector3f randVec3(float low, float high) {
   return Vector3f(randRange(low, high), randRange(low, high), randRange(low, high));
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

static float square(float f) {
   return f * f;
}

template <typename T>
static int pntrToNdx(std::vector<T>& cont, T * elem) {
   return elem - &cont[0];
}

Model * TerrainGenerator::GenerateModel() {
   edgeLength = 1;

   grid = new SpatialGrid(1000, 2 * edgeLength);

   // The general direction the mesh is heading towards
   Vector3f ultimateDirection = Vector3f(0.25,1.0,-0.25).normalized();

   Vector3f bitangent = ultimateDirection;
   Vector3f tangent = crossWithUp(bitangent);
   Vector3f normal = tangent.cross(bitangent).normalized();
   float halfLength = edgeLength / 2.0f;

   // Set up the beginning vertices
   Vertex vTop;
   vTop.normal = normal;
   vTop.tangent = tangent;
   vTop.bitangent = bitangent;
   vTop.position = halfLength * bitangent;
   vTop.uv = Vector2f(0.5f, 1.0f);

   Vertex vLeft;
   vLeft.normal = normal;
   vLeft.tangent = tangent;
   vLeft.bitangent = bitangent;
   vLeft.position = -halfLength * bitangent - halfLength * tangent;
   vLeft.uv = Vector2f(0.0f, 0.0f);

   Vertex vRight;
   vRight.normal = normal;
   vRight.tangent = tangent;
   vRight.bitangent = bitangent;
   vRight.position = -halfLength * bitangent + halfLength * tangent;
   vRight.uv = Vector2f(1.0f, 0.0f);

   Face face;
   face.vertIndices[0] = 0;
   face.vertIndices[1] = 1;
   face.vertIndices[2] = 2;
   face.normal = normal;

   // Initialize the model
   model = new Model();
   model->vertices = std::vector<Vertex>(0);
   model->vertices.reserve(1000000);
   model->faces = std::vector<Face>(0);
   model->faces.reserve(1000000);

   // Add the 3 vertices to the model list and the grid
   model->vertices.push_back(vTop);
   grid->Add(& model->vertices.back());
   model->vertices.push_back(vLeft);
   grid->Add(& model->vertices.back());
   model->vertices.push_back(vRight);
   grid->Add(& model->vertices.back());

   model->faces.push_back(face);
   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;
   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   // Send the triangle data to the shader
   model->bufferVertices();
   model->bufferIndices();

   // Create the first 6 paths
   paths = std::vector<Path *>(0);
   Path * p;

   p = new Path();
   p->headV = & model->vertices[0];
   p->tailV = & model->vertices[1];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[0];
   p->tailV = & model->vertices[2];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[1];
   p->tailV = & model->vertices[2];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[1];
   p->tailV = & model->vertices[0];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[2];
   p->tailV = & model->vertices[0];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[2];
   p->tailV = & model->vertices[1];
   paths.push_back(p);

   paths[0]->leftP = paths[1];
   paths[0]->rightP = paths[5];
   paths[1]->leftP = paths[2];
   paths[1]->rightP = paths[0];
   paths[2]->leftP = paths[3];
   paths[2]->rightP = paths[1];
   paths[3]->leftP = paths[4];
   paths[3]->rightP = paths[2];
   paths[4]->leftP = paths[5];
   paths[4]->rightP = paths[3];
   paths[5]->leftP = paths[0];
   paths[5]->rightP = paths[4];

   stepCnt = 0;

   return model;
}

void TerrainGenerator::BuildStep() {
   int numPaths = paths.size();

   // Extend each path to build the vertices
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      Vector3f randDir = (randRange(-0.2, 0.2) * p->headV->normal);
      Vector3f heading = edgeLength * (p->headV->position + randDir - p->tailV->position).normalized();

      // Add vertex created from extending the path
      Vertex v;
      v.position = p->headV->position + heading;
      v.tangent = p->headV->tangent;
      v.bitangent = p->headV->bitangent;
      v.normal = p->headV->normal;
      Matrix3f iTBN = Mmath::InverseTBN(v.tangent, v.bitangent, v.normal);
      v.uv = p->headV->uv + (iTBN * heading).head<2>();

      // Instead of adding vertices here, put in a new list and add them after merging paths
      model->vertices.push_back(v);
      grid->Add(& model->vertices.back());

      // Update the path head and tail vertices
      p->tailV = p->headV;
      p->headV = & model->vertices.back();
   }

   // numPaths = paths.size();

   // // Merge paths that are too near each other
   // for (int i = 0; i < numPaths; i++) {

   // }

   numPaths = paths.size();

   // Go through each path to create new faces
   for (int i = 0; i < numPaths; i++) {
      Path * selfP = paths[i];
      Path * rightP = selfP->rightP;

      if (selfP->tailV == rightP->tailV)
         HandleSameTail(selfP, rightP);
      else
         HandleDiffTail(selfP, rightP);
   }

   model->CalculateNormals();
   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   model->bufferVertices();
   model->bufferIndices();

   stepCnt++;
   printf("Step [%d]: paths %d, verts %d faces %d\n", stepCnt, paths.size(), model->vertices.size(), model->faces.size());
}

void TerrainGenerator::HandleSameTail(Path * leftP, Path * rightP) {
   // Create the emerging face
   Face f;
   f.vertIndices[0] = pntrToNdx(model->vertices, leftP->headV);
   f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
   f.vertIndices[2] = pntrToNdx(model->vertices, rightP->headV);
   model->faces.push_back(f);
}

void TerrainGenerator::HandleDiffTail(Path * leftP, Path * rightP) {
   float headDistSq = (leftP->headV->position - rightP->headV->position).squaredNorm();
   float maxEdgeLen = square(1.5 * edgeLength);

   if (headDistSq < maxEdgeLen)
      HandleOKHeadDist(leftP, rightP);
   else
      HandleBigHeadDist(leftP, rightP);
}

void TerrainGenerator::HandleOKHeadDist(Path * leftP, Path * rightP) {
   float bltrDistSq = (rightP->headV->position - leftP->tailV->position).squaredNorm();
   float brtlDistSq = (leftP->headV->position - rightP->tailV->position).squaredNorm();
   Face f;

   // Create 2 faces to fill the space
   if (bltrDistSq < brtlDistSq) {
      f.vertIndices[0] = pntrToNdx(model->vertices, leftP->headV);
      f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
      f.vertIndices[2] = pntrToNdx(model->vertices, rightP->headV);
      model->faces.push_back(f);

      f.vertIndices[0] = pntrToNdx(model->vertices, rightP->headV);
      f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
      f.vertIndices[2] = pntrToNdx(model->vertices, rightP->tailV);
      model->faces.push_back(f);
   } else {
      f.vertIndices[0] = pntrToNdx(model->vertices, leftP->headV);
      f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
      f.vertIndices[2] = pntrToNdx(model->vertices, rightP->tailV);
      model->faces.push_back(f);

      f.vertIndices[0] = pntrToNdx(model->vertices, rightP->headV);
      f.vertIndices[1] = pntrToNdx(model->vertices, leftP->headV);
      f.vertIndices[2] = pntrToNdx(model->vertices, rightP->tailV);
      model->faces.push_back(f);
   }
}

void TerrainGenerator::HandleBigHeadDist(Path * leftP, Path * rightP) {
   // Add a vertex between them
   Vertex v;
   v.position = 0.5f * (leftP->headV->position + rightP->headV->position);
   v.tangent = leftP->tailV->tangent;
   v.bitangent = leftP->tailV->bitangent;
   v.normal = leftP->tailV->normal;
   Matrix3f iTBN = Mmath::InverseTBN(v.tangent, v.bitangent, v.normal);
   v.uv = leftP->tailV->uv + (iTBN * (v.position - leftP->tailV->position)).head<2>();
   model->vertices.push_back(v);
   grid->Add(& model->vertices.back());

   Vertex * midV = & model->vertices.back();

   // Create 2 new paths who's head is the new vertex
   Path * midRightP = new Path();
   Path * midLeftP = new Path();

   midRightP->headV = midV;
   midRightP->tailV = leftP->tailV;
   midRightP->leftP = midLeftP;
   midRightP->rightP = rightP;

   midLeftP->headV = midV;
   midLeftP->tailV = rightP->tailV;
   midLeftP->leftP = leftP;
   midLeftP->rightP = midRightP;

   paths.push_back(midRightP);
   paths.push_back(midLeftP);

   // Update the paths to the left and right of the created ones
   leftP->rightP = midLeftP;
   rightP->leftP = midRightP;

   // Create the emerging triangle faces
   Face f;
   f.vertIndices[0] = pntrToNdx(model->vertices, leftP->headV);
   f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
   f.vertIndices[2] = pntrToNdx(model->vertices, midV);
   model->faces.push_back(f);

   f.vertIndices[0] = pntrToNdx(model->vertices, midV);
   f.vertIndices[1] = pntrToNdx(model->vertices, rightP->tailV);
   f.vertIndices[2] = pntrToNdx(model->vertices, rightP->headV);
   model->faces.push_back(f);

   f.vertIndices[0] = pntrToNdx(model->vertices, midV);
   f.vertIndices[1] = pntrToNdx(model->vertices, leftP->tailV);
   f.vertIndices[2] = pntrToNdx(model->vertices, rightP->tailV);
   model->faces.push_back(f);
}

VertDist FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist) {
   grid->FindClosestVertex(targetPnt, maxDist);
}
