
#include "terrain.h"
#include "stdio.h"
#include "assert.h"

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

static Vector3f directionFromPoints(Vector3f head, Vector3f tail) {
   Vector3f diff = head - tail;
   if ((!diff(0) && !diff(1) && !diff(2)) || diff(0) > 10000 || diff(0) < -10000 || diff(1) > 10000 || diff(1) < -10000 || diff(2) > 10000 || diff(2) < -10000)
      printf("diff = %f %f %f\n", diff(0), diff(1), diff(2));
   return diff.normalized();
}

void TerrainGenerator::Path::CalculateHeading() {
   heading = directionFromPoints(headV->position, tailV->position);
}

Vector3f TerrainGenerator::Path::getPosition() {
   return headV->getPosition();
}

Model * TerrainGenerator::GenerateModel() {
   edgeLength = 1;

   // grid = new SpatialGrid(1000, 2 * edgeLength);

   // The general direction the mesh is heading towards
   Vector3f ultimateDirection = Vector3f(0.25,1.0,-0.25).normalized();

   Vector3f bitangent = ultimateDirection;
   Vector3f tangent = crossWithUp(bitangent);
   Vector3f normal = tangent.cross(bitangent).normalized();

   // Set up the first vertex
   Vertex * vStart = new Vertex();
   vStart->index = 0;
   vStart->normal = normal;
   vStart->tangent = tangent;
   vStart->bitangent = bitangent;
   vStart->position = Vector3f(0,0,0);
   vStart->uv = Vector2f(0.5f, 0.5f);

   // Initialize the model
   model = new Model();
   model->vertices = std::vector<Vertex *>(0);
   model->faces = std::vector<Face *>(0);

   // Add the vertex to the model and grid
   model->vertices.push_back(vStart);
   // grid->Add(vStart);

   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;

   // Initialize the first paths
   int numInitPaths = 6;
   paths = std::vector<Path *>(0);

   for (int i = 0; i < numInitPaths; i++)
      paths.push_back(new Path());
   for (int i = 0; i < numInitPaths; i++) {
      paths[i]->headV = vStart;
      paths[i]->leftP = paths[(i+1) % numInitPaths];
      paths[i]->rightP = paths[(i-1 + numInitPaths) % numInitPaths];
      paths[i]->heading = (cos(2*M_PI*i/numInitPaths) * bitangent - sin(2*M_PI*i/numInitPaths) * tangent).normalized();
      paths[i]->extending = true;
   }

   ExtendPaths();
   SmoothPathPositions();
   MergePaths();
   CreateNeededPaths();
   AddVerticesAndFaces();
   RemoveConvergingPaths();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->CalculateNormals();
   model->bufferVertices();
   model->bufferIndices();

   return model;
}

void TerrainGenerator::BuildStep(Vector3f center, float radius) {

   PickPathsToExtend(center, radius);
   ExtendPaths();
   SmoothPathPositions();
   MergePaths();
   CreateNeededPaths();
   AddVerticesAndFaces();
   RemoveConvergingPaths();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->CalculateNormals();
   model->bufferVertices();
   model->bufferIndices();

   printf("Step: paths %d, verts %d, faces %d, center (%.2f, %.2f, %.2f)\n", paths.size(), model->vertices.size(), model->faces.size(), center(0), center(1), center(2));
}

void TerrainGenerator::PickPathsToExtend(Vector3f center, float radius) {
   int numPaths = paths.size();
   float radiusSq = radius * radius;
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      float distFromCenterSq = (p->headV->position - center).squaredNorm();
      p->extending = distFromCenterSq < radiusSq;
   }
}

void TerrainGenerator::ExtendPaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      if (p->extending) {
         Vector3f randY = randRange(-0.1, 0.1) * p->headV->normal;
         Vector3f newPos = p->headV->position + edgeLength * (p->heading + randY).normalized();

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
}

void TerrainGenerator::SmoothPathPositions() {
   int numPaths = paths.size();
   std::vector<Vector3f> updatedPositions = std::vector<Vector3f>(numPaths);

   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      if (p->extending) {
         Vector3f midTail = 0.5f * (p->leftP->tailV->position + p->rightP->tailV->position);
         Vector3f midHead = 0.5f * (p->leftP->headV->position + p->rightP->headV->position);
         Vector3f midDir = directionFromPoints(midHead, midTail);
         Vector3f properPos = midTail + edgeLength * midDir;
         updatedPositions[i] = 0.25f * p->headV->position + 0.75f * properPos;
      }
   }

   for (int i = 0; i < numPaths; i++)
      if (paths[i]->extending)
         paths[i]->headV->position = updatedPositions[i];
}

void TerrainGenerator::MergePaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * midP = paths[i];
      Path * rightP = midP->rightP;
      Vertex * keepV = midP->headV;
      Vertex * removeV = rightP->headV;

      float headDistSq = (keepV->position - removeV->position).squaredNorm();
      float minDistSq = square(0.6f * edgeLength);

      // If distance between the heads is too small
      if ( midP->extending && rightP->extending &&
           midP->tailV != rightP->tailV &&
           headDistSq < minDistSq ) {
         // Delete the right vertex and set all paths that had this vertex to the mid vertex
         Path * curP = rightP;
         int numConverging = 1;
         while (curP->headV == removeV) {
            keepV->position += curP->headV->position;
            curP->headV = keepV;
            numConverging++;
            curP = curP->rightP;
         }

         keepV->position = (1.0f/numConverging) * keepV->position;
         delete(removeV);
      }
   }
}

void TerrainGenerator::CreateNeededPaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * outLeftP = paths[i];
      Path * outRightP = outLeftP->rightP;

      if (outLeftP->extending && outRightP->extending) {
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

            float distSqL = (midV->position - outLeftP->tailV->position).squaredNorm();
            float distSqR = (midV->position - outRightP->tailV->position).squaredNorm();

            // Create a new path
            Path * midP = new Path();
            midP->headV = midV;
            midP->tailV = distSqL < distSqR ? outLeftP->tailV : outRightP->tailV;
            midP->leftP = outLeftP;
            midP->rightP = outRightP;
            midP->extending = true;

            Vector3f midTailPos = 0.5f * (outLeftP->tailV->position + outRightP->tailV->position);
            midP->heading = directionFromPoints(midP->headV->position, midTailPos);
            paths.push_back(midP);

            // Update outer path references
            outLeftP->rightP = midP;
            outRightP->leftP = midP;
         }
      }
   }
}

void TerrainGenerator::AddVerticesAndFaces() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * selfP = paths[i];
      Path * rightP = selfP->rightP;

      // Add the vertex
      if ( selfP->extending && selfP->headV != rightP->headV) {
         selfP->headV->index = model->vertices.size();
         model->vertices.push_back(selfP->headV);
         // grid->Add(selfP->headV);
      }

      // Add the faces
      if (selfP->headV == rightP->headV)
         HandleSameHead(selfP, rightP);
      else if (selfP->tailV == rightP->tailV)
         HandleSameTail(selfP, rightP);
      else if (selfP->headV != rightP->headV && selfP->tailV != rightP->tailV)
         HandleBothDiff(selfP, rightP);
      else
         printf("Overlapping path at %d. woops...\n", i);
   }
}

void TerrainGenerator::HandleSameHead(Path * leftP, Path * rightP) {
   if (leftP->extending && rightP->extending) {
      // Create the emerging face
      Face * f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->tailV;
      model->faces.push_back(f);
   }
}

void TerrainGenerator::HandleSameTail(Path * leftP, Path * rightP) {
   if (leftP->extending) {
      // Create the emerging face
      Face * f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);
   }
}

void TerrainGenerator::HandleBothDiff(Path * leftP, Path * rightP) {
   float bltrDistSq = (rightP->headV->position - leftP->tailV->position).squaredNorm();
   float brtlDistSq = (leftP->headV->position - rightP->tailV->position).squaredNorm();
   Face * f;

   // Create faces to fill the space
   if (leftP->extending && !rightP->extending) {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);
   } else if (!leftP->extending && rightP->extending) {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = rightP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);
   } else if (leftP->extending && rightP->extending) {
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
}

void TerrainGenerator::RemoveConvergingPaths() {
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

PointDist TerrainGenerator::FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist) {
   return grid->FindClosest(targetPnt, maxDist);
}
