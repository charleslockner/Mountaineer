
#include "terrain.h"
#include "stdio.h"

#include <algorithm>

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

void TerrainGenerator::Path::CalculateHeading() {
   heading = (headV->position - tailV->position).normalized();
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
   Vertex * vTop = new Vertex();
   vTop->index = 0;
   vTop->normal = normal;
   vTop->tangent = tangent;
   vTop->bitangent = bitangent;
   vTop->position = halfLength * bitangent;
   vTop->uv = Vector2f(0.5f, 1.0f);

   Vertex * vLeft = new Vertex();
   vLeft->index = 1;
   vLeft->normal = normal;
   vLeft->tangent = tangent;
   vLeft->bitangent = bitangent;
   vLeft->position = -halfLength * bitangent - halfLength * tangent;
   vLeft->uv = Vector2f(0.0f, 0.0f);

   Vertex * vRight = new Vertex();
   vRight->index = 2;
   vRight->normal = normal;
   vRight->tangent = tangent;
   vRight->bitangent = bitangent;
   vRight->position = -halfLength * bitangent + halfLength * tangent;
   vRight->uv = Vector2f(1.0f, 0.0f);

   Face * face = new Face();;
   face->vertices[0] = vTop;
   face->vertices[1] = vLeft;
   face->vertices[2] = vRight;
   face->normal = normal;

   // Initialize the model
   model = new Model();
   model->vertices = std::vector<Vertex *>(0);
   model->faces = std::vector<Face *>(0);

   // Add the 3 vertices to the model list and the grid
   model->vertices.push_back(vTop);
   model->vertices.push_back(vLeft);
   model->vertices.push_back(vRight);
   model->faces.push_back(face);
   grid->Add(vTop);
   grid->Add(vLeft);
   grid->Add(vRight);

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
   p->headV = model->vertices[0];
   p->tailV = model->vertices[1];
   p->heading = (p->headV->position - p->tailV->position).normalized();
   paths.push_back(p);
   p = new Path();
   p->headV = model->vertices[0];
   p->tailV = model->vertices[2];
   p->heading = (p->headV->position - p->tailV->position).normalized();
   paths.push_back(p);
   p = new Path();
   p->headV = model->vertices[1];
   p->tailV = model->vertices[2];
   p->heading = (p->headV->position - p->tailV->position).normalized();
   paths.push_back(p);
   p = new Path();
   p->headV = model->vertices[1];
   p->tailV = model->vertices[0];
   p->heading = (p->headV->position - p->tailV->position).normalized();
   paths.push_back(p);
   p = new Path();
   p->headV = model->vertices[2];
   p->tailV = model->vertices[0];
   p->heading = (p->headV->position - p->tailV->position).normalized();
   paths.push_back(p);
   p = new Path();
   p->headV = model->vertices[2];
   p->tailV = model->vertices[1];
   p->heading = (p->headV->position - p->tailV->position).normalized();
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

   ExtendPaths();
   MergePathHeads();
   AddNeededPaths();
   AddVertices();
   CreateFaces();
   RemoveCrossPaths();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->CalculateNormals();
   model->bufferVertices();
   model->bufferIndices();

   stepCnt++;
   printf("Step [%d]: paths %d, verts %d, faces %d\n", stepCnt, paths.size(), model->vertices.size(), model->faces.size());
}

void TerrainGenerator::ExtendPaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      Vector3f randDir = (randRange(-0.1, 0.1) * p->headV->normal);
      Vector3f leftPos = p->leftP->headV->position + edgeLength * p->leftP->heading;
      Vector3f rightPos = p->rightP->headV->position + edgeLength * p->rightP->heading;
      Vector3f midPnt = 0.5f * (leftPos + rightPos);
      Vector3f newPos = 0.5f * (midPnt + (p->headV->position + edgeLength * (p->heading + randDir)));

      // Add vertex created from extending the path
      Vertex * v = new Vertex();
      v->position = newPos;
      v->tangent = p->headV->tangent;
      v->bitangent = p->headV->bitangent;
      v->normal = p->headV->normal;

      Matrix3f iTBN = Mmath::InverseTBN(v->tangent, v->bitangent, v->normal);
      v->uv = p->headV->uv + (iTBN * (newPos - p->headV->position)).head<2>();

      // Update the path
      p->tailV = p->headV;
      p->headV = v;
      p->CalculateHeading();
   }
}

void TerrainGenerator::MergePathHeads() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * midP = paths[i];
      Path * rightP = midP->rightP;

      float headDistSq = (midP->headV->position - rightP->headV->position).squaredNorm();
      float minDistSq = square(0.6f * edgeLength);

      // If distance between the heads is too small
      if (midP->tailV != rightP->tailV && headDistSq < minDistSq) {
         // merge the right path head with the middle
         midP->headV->position = 0.5f * (midP->headV->position + rightP->headV->position);
         rightP->headV = midP->headV;
         delete(rightP->headV);
      }
   }
}

void TerrainGenerator::AddNeededPaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * outLeftP = paths[i];
      Path * outRightP = outLeftP->rightP;

      float headDistSq = (outLeftP->headV->position - outRightP->headV->position).squaredNorm();
      float maxEdgeLen = square(1.5 * edgeLength);

      if (headDistSq > maxEdgeLen) {
         // Add a vertex between them
         Vertex * midV = new Vertex();
         midV->position = 0.5f * (outLeftP->headV->position + outRightP->headV->position);
         midV->tangent = outLeftP->tailV->tangent;
         midV->bitangent = outLeftP->tailV->bitangent;
         midV->normal = outLeftP->tailV->normal;

         Matrix3f iTBN = Mmath::InverseTBN(midV->tangent, midV->bitangent, midV->normal);
         midV->uv = outLeftP->tailV->uv + (iTBN * (midV->position - outLeftP->tailV->position)).head<2>();

         // Create 2 new paths who's head is the new vertex
         Path * midLeftP = new Path();
         Path * midRightP = new Path();

         midLeftP->headV = midV;
         midLeftP->tailV = outLeftP->tailV;
         midLeftP->leftP = outLeftP;
         midLeftP->rightP = midRightP;
         midLeftP->CalculateHeading();

         midRightP->headV = midV;
         midRightP->tailV = outRightP->tailV;
         midRightP->leftP = midLeftP;
         midRightP->rightP = outRightP;
         midRightP->CalculateHeading();

         paths.push_back(midLeftP);
         paths.push_back(midRightP);

         // Update outer path references
         outLeftP->rightP = midLeftP;
         outRightP->leftP = midRightP;
      }
   }
}

void TerrainGenerator::AddVertices() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      if (paths[i]->headV != paths[i]->rightP->headV) {
         Vertex * v = paths[i]->headV;
         v->index = model->vertices.size();
         model->vertices.push_back(v);
         grid->Add(v);
      }
   }
}

void TerrainGenerator::CreateFaces() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * selfP = paths[i];
      Path * rightP = selfP->rightP;

      if (selfP->headV == rightP->headV && selfP->tailV == rightP->tailV)
         printf("Overlapping path at %d. woops...\n", i);
      else if (selfP->headV == rightP->headV)
         HandleSameHead(selfP, rightP);
      else if (selfP->tailV == rightP->tailV)
         HandleSameTail(selfP, rightP);
      else
         HandleBothDiff(selfP, rightP);
   }
}

void TerrainGenerator::HandleSameHead(Path * leftP, Path * rightP) {
   // Create the emerging face
   Face * f = new Face();
   f->vertices[0] = leftP->headV;
   f->vertices[1] = leftP->tailV;
   f->vertices[2] = rightP->tailV;
   model->faces.push_back(f);
}

void TerrainGenerator::HandleSameTail(Path * leftP, Path * rightP) {
   // Create the emerging face
   Face * f = new Face();
   f->vertices[0] = leftP->headV;
   f->vertices[1] = leftP->tailV;
   f->vertices[2] = rightP->headV;
   model->faces.push_back(f);
}

void TerrainGenerator::HandleBothDiff(Path * leftP, Path * rightP) {
   float bltrDistSq = (rightP->headV->position - leftP->tailV->position).squaredNorm();
   float brtlDistSq = (leftP->headV->position - rightP->tailV->position).squaredNorm();
   Face * f;

   // Create 2 faces to fill the space
   if (bltrDistSq < brtlDistSq) {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);

      f = new Face();
      f->vertices[0] = rightP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->tailV;
      model->faces.push_back(f);
   } else {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->tailV;
      model->faces.push_back(f);

      f = new Face();
      f->vertices[0] = rightP->headV;
      f->vertices[1] = leftP->headV;
      f->vertices[2] = rightP->tailV;
      model->faces.push_back(f);
   }
}

void TerrainGenerator::RemoveCrossPaths() {
   for (int i = 0; i < paths.size(); i++) {
      Path * midP = paths[i];
      Path * leftP = midP->leftP;
      Path * rightP = midP->rightP;

      if (midP->headV == rightP->headV) {
         // Fix the left/right paths of the neighbors
         rightP->heading = (midP->heading + rightP->heading).normalized();
         leftP->rightP = rightP;
         rightP->leftP = leftP;

         // Remove the mid path
         delete(paths[i]);
         paths.erase(paths.begin() + i);
         i--;
      }
   }
}

VertDist TerrainGenerator::FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist) {
   return grid->FindClosest(targetPnt, maxDist);
}
