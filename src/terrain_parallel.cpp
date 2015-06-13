
#include "terrain.h"
#include "reducer.h"

#include <iostream>
#include <assert.h>
#include <algorithm>
#include <sys/time.h>

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/tbb_allocator.h"
#include "tbb/atomic.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_sort.h"
#include "tbb/concurrent_vector.h"
#include "tbb/mutex.h"

#define MAX_FIND_DIST 40
#define NUM_INIT_PATHS 6

using namespace Eigen;

// ============================================================ //
// ===================== STATIC FUNCTIONS ===================== //
// ============================================================ //

static double randRange(float low, float high) {
   return (high - low) * rand() / (float)RAND_MAX + low;
}

static Vector3f randVec3(float low, float high) {
   return Vector3f(randRange(low, high), randRange(low, high), randRange(low, high));
}

static inline float square(float f) {
   return f * f;
}

static Vector3f directionFromPoints(Vector3f head, Vector3f tail) {
   Vector3f diff = head - tail;
   if ((!diff(0) && !diff(1) && !diff(2)) || diff(0) > 10000 || diff(0) < -10000 || diff(1) > 10000 || diff(1) < -10000 || diff(2) > 10000 || diff(2) < -10000)
      printf("diff = %f %f %f\n", diff(0), diff(1), diff(2));
   return diff.normalized();
}

static void makeNeighbors(Vertex * a, Vertex * b) {
   a->neighbors.push_back(b);
   b->neighbors.push_back(a);
}

static void addFaceToVertexReferences(Face * f) {
   f->vertices[0]->faces.push_back(f);
   f->vertices[1]->faces.push_back(f);
   f->vertices[2]->faces.push_back(f);
}

static Vertex * neighborFromDirection(Vertex * baseV, Vector3f dir) {
   int numNeighs = baseV->neighbors.size();
   if (numNeighs == 0) {
      printf("Waht are you doing!??\n");
      return NULL;
   }

   float largestDot = dir.dot(directionFromPoints(baseV->neighbors[0]->position, baseV->position));
   Vertex * bestVert = baseV->neighbors[0];

   for (int i = 1; i < numNeighs; i++) {
      float dot = dir.dot(directionFromPoints(baseV->neighbors[i]->position, baseV->position));
      if (dot > largestDot) {
         largestDot = dot;
         bestVert = baseV->neighbors[i];
      }
   }
   return bestVert;
}

long long last = 0;

static void startTimer() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   last = (long long) 1000000 * tp.tv_sec + tp.tv_usec;
   std::cout << "Starting timer\n";
}

static void printTimeDelta() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   long long curTime = (long long) 1000000 * tp.tv_sec + tp.tv_usec; //get current timestamp in milliseconds
   long long timeDiff = curTime - last;
   last = curTime;
   std::cout << "time since last = " << timeDiff << std::endl;
}

// ============================================================ //
// ===================== PUBLIC FUNCTIONS ===================== //
// ============================================================ //

TerrainGenerator::TerrainGenerator() {};

Model * TerrainGenerator::GenerateModel() {
   edgeLength = 1;

   // The general direction the mesh is heading towards
   Vector3f ultimateDirection = Vector3f(0.25,1.0,-0.25).normalized();

   Vector3f bitangent = ultimateDirection;
   Vector3f tangent = bitangent.cross(Vector3f(0,1,0)).normalized();
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
   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;

   // Add the vertex to the model
   model->vertices.push_back(vStart);

   // Initialize the starting paths
   paths = std::vector<Path *>(0);
   for (int i = 0; i < NUM_INIT_PATHS; i++)
      paths.push_back(new Path());
   for (int i = 0; i < NUM_INIT_PATHS; i++) {
      paths[i]->headV = vStart;
      paths[i]->leftP = paths[(i+1) % NUM_INIT_PATHS];
      paths[i]->rightP = paths[(i-1 + NUM_INIT_PATHS) % NUM_INIT_PATHS];
      paths[i]->heading = (cos(2*M_PI*i/NUM_INIT_PATHS) * bitangent - sin(2*M_PI*i/NUM_INIT_PATHS) * tangent).normalized();
      paths[i]->buildAction = Path::BuildAction::ADVANCE;
   }

   BuildStep();

   return model;
}

void TerrainGenerator::UpdateMesh(Vector3f center, float radius) {
   PickPathsToExtend(center, radius);
   BuildStep();
}

// ============================================================ //
// ===================== PRIVATE FUNCTIONS ==================== //
// ============================================================ //

void TerrainGenerator::BuildStep() {
   ExtendPaths();
   MergePaths();
   CreateNeededPaths();
   AddVerticesAndFaces();
   RemoveRetreatingGeometry();
   RemoveConvergingPaths();
   CalculateVertexNormals();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->bufferVertices();
   model->bufferIndices();
   // printf("Step: paths %d, verts %d, faces %d, center (%.2f, %.2f, %.2f)\n", paths.size(), model->vertices.size(), model->faces.size(), center(0), center(1), center(2));
}

void TerrainGenerator::PickPathsToExtend(Vector3f center, float radius) {
   int numPaths = paths.size();
   float radiusSq = radius * radius;

   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];
      float headDistSq = (p->headV->position - center).squaredNorm();
      float tailDistSq = (p->tailV->position - center).squaredNorm();

      if (headDistSq < radiusSq)
         p->buildAction = Path::BuildAction::ADVANCE;
      else if (headDistSq > radiusSq && tailDistSq > radiusSq)
         p->buildAction = Path::BuildAction::RETREAT;
      else
         p->buildAction = Path::BuildAction::STATION;
   });
}

void TerrainGenerator::ExtendPaths() {
   int numPaths = paths.size();

   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];

      if (p->buildAction == Path::BuildAction::ADVANCE) {
         // printf("Advancing path %d\n", i);

         Vector3f randY = randRange(-0.01, 0.01) * p->headV->normal;

         // Add vertex created from extending the path
         Vertex * v = new Vertex();
         v->position = p->headV->position + edgeLength * (p->heading).normalized();
         v->tangent = p->headV->tangent;
         v->bitangent = p->headV->bitangent;
         v->normal = p->headV->normal;

         // Update the path
         p->tailV = p->headV;
         p->headV = v;

         p->calculateHeading();
      }
   });

   // Smooth the positions of the newly created vertices
   std::vector<Vector3f> updatedPositions = std::vector<Vector3f>(numPaths);
   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];

      if (p->buildAction == Path::BuildAction::ADVANCE) {
         Vector3f midTail = 0.5f * (p->leftP->tailV->position + p->rightP->tailV->position);
         Vector3f midHead = 0.5f * (p->leftP->headV->position + p->rightP->headV->position);
         Vector3f midDir = directionFromPoints(midHead, midTail);
         Vector3f properPos = midTail + edgeLength * midDir;
         updatedPositions[i] = 0.25f * p->headV->position + 0.75f * properPos;
      }
   });

   // Update the position and uv coords
   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];
      if (paths[i]->buildAction == Path::BuildAction::ADVANCE)
         p->headV->position = updatedPositions[i];
      p->calculateUV();
   });
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

      // If there is not enough distance between the heads
      if ( midP->buildAction == Path::BuildAction::ADVANCE &&
           rightP->buildAction == Path::BuildAction::ADVANCE &&
           midP->tailV != rightP->tailV &&
           headDistSq < minDistSq ) {

         // Average the positions of the heads
         Path * curP = rightP;
         int numConverging = 1;
         while (curP->headV == removeV) {
            keepV->position += curP->headV->position;
            curP->headV = keepV;
            numConverging++;
            curP = curP->rightP;
         }
         keepV->position = (1.0f/numConverging) * keepV->position;

         // Delete the vertex that all the paths to the right had as a head
         delete(removeV);
      }
   }
}

void TerrainGenerator::CreateNeededPaths() {
   int numPaths = paths.size();
   for (int i = 0; i < numPaths; i++) {
      Path * outLeftP = paths[i];
      Path * outRightP = outLeftP->rightP;

      if (outLeftP->buildAction == Path::BuildAction::ADVANCE &&
          outRightP->buildAction == Path::BuildAction::ADVANCE) {
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
            midV->uv = outLeftP->tailV->uv + UV_STEP_SIZE * (iTBN * (midV->position - outLeftP->tailV->position)).head<2>();

            float distSqL = (midV->position - outLeftP->tailV->position).squaredNorm();
            float distSqR = (midV->position - outRightP->tailV->position).squaredNorm();

            // Create a new path
            Path * midP = new Path();
            midP->headV = midV;
            midP->tailV = distSqL < distSqR ? outLeftP->tailV : outRightP->tailV;
            midP->leftP = outLeftP;
            midP->rightP = outRightP;
            midP->buildAction = Path::BuildAction::ADVANCE;

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
      if (selfP->buildAction == Path::BuildAction::ADVANCE &&
          selfP->headV != rightP->headV) {
         selfP->headV->index = model->vertices.size();
         model->vertices.push_back(selfP->headV);
      }

      // Add the faces and neighbors
      if (selfP->headV == rightP->headV)
         HandleSameHead(selfP, rightP);
      else if (selfP->tailV == rightP->tailV)
         HandleSameTail(selfP, rightP);
      else if (selfP->headV != rightP->headV && selfP->tailV != rightP->tailV)
         HandleBothDiff(selfP, rightP);
      else {
         printf("Overlapping path at %d. woops...\n", i);
         exit(1);
      }
   }
}

// Note the mid path always adds neighbors between it's own head and tail
// The right path head and tail should be added as neighbors yet because of this.
// The current path adds neighbors between the itself and right path vertices.
// Because of this, do not add neighbor pairs between mid and left path vertices

void TerrainGenerator::HandleSameHead(Path * midP, Path * rightP) {
   if (midP->buildAction == Path::BuildAction::ADVANCE &&
       rightP->buildAction == Path::BuildAction::ADVANCE) {
      // Create the emerging face
      Face * f = new Face();
      f->vertices[0] = midP->headV;
      f->vertices[1] = midP->tailV;
      f->vertices[2] = rightP->tailV;
      f->calculateNormal();
      model->faces.push_back(f);
      addFaceToVertexReferences(f);

      // Add neighbors to all effected vertices
      makeNeighbors(midP->tailV, midP->headV);
      // Don't add between path->head and rightP->tailV, since this is done by another iteration
   }
}

void TerrainGenerator::HandleSameTail(Path * midP, Path * rightP) {
   if (midP->buildAction == Path::BuildAction::ADVANCE) {
      // Create the emerging face
      Face * f = new Face();
      f->vertices[0] = midP->headV;
      f->vertices[1] = midP->tailV;
      f->vertices[2] = rightP->headV;
      f->calculateNormal();
      model->faces.push_back(f);
      addFaceToVertexReferences(f);

      // Add neighbors to all effected vertices
      makeNeighbors(midP->tailV, midP->headV);
      makeNeighbors(midP->headV, rightP->headV);
      // Don't add between tail and rightP->headV, since this is done by another iteration
   }
}

void TerrainGenerator::HandleBothDiff(Path * midP, Path * rightP) {
   Face * f;

   // Handle the edge case where the mid path is advancing but not the right path
   if (midP->buildAction == Path::BuildAction::ADVANCE &&
       rightP->buildAction != Path::BuildAction::ADVANCE) {
      // Create the emerging face
      f = new Face();
      f->vertices[0] = midP->headV;
      f->vertices[1] = midP->tailV;
      f->vertices[2] = rightP->headV;
      f->calculateNormal();
      model->faces.push_back(f);
      addFaceToVertexReferences(f);

      // Add in the neighbors
      makeNeighbors(midP->tailV, midP->headV);
      makeNeighbors(midP->headV, rightP->headV);
   }
   // Handle the edge case where the right path is advancing but not the mid path
   else if (midP->buildAction != Path::BuildAction::ADVANCE &&
              rightP->buildAction == Path::BuildAction::ADVANCE) {
      // Create the emerging face
      f = new Face();
      f->vertices[0] = midP->headV;
      f->vertices[1] = rightP->tailV;
      f->vertices[2] = rightP->headV;
      f->calculateNormal();
      model->faces.push_back(f);
      addFaceToVertexReferences(f);

      // Add in the neighbors
      makeNeighbors(midP->headV, rightP->headV);
      // Don't add rightP->headV, rightP->tailV, since this is added by another iteration
   }
   // Handle the case when both paths are advancing
   else if (midP->buildAction == Path::BuildAction::ADVANCE &&
              rightP->buildAction == Path::BuildAction::ADVANCE) {

      float bltrDistSq = (rightP->headV->position - midP->tailV->position).squaredNorm();
      float brtlDistSq = (midP->headV->position - rightP->tailV->position).squaredNorm();

      // Avoid creating skinny triangles by splitting the square with the closest opposing corners
      if (bltrDistSq < brtlDistSq) {
         // Create the emerging faces
         f = new Face();
         f->vertices[0] = midP->headV;
         f->vertices[1] = midP->tailV;
         f->vertices[2] = rightP->headV;
         f->calculateNormal();
         model->faces.push_back(f);
         addFaceToVertexReferences(f);

         f = new Face();
         f->vertices[0] = rightP->headV;
         f->vertices[1] = midP->tailV;
         f->vertices[2] = rightP->tailV;
         f->calculateNormal();
         model->faces.push_back(f);
         addFaceToVertexReferences(f);

         // Add in the neighbors
         makeNeighbors(midP->tailV, midP->headV);
         makeNeighbors(midP->tailV, rightP->headV);
         makeNeighbors(midP->headV, rightP->headV);

      } else {
         // Create the emerging faces
         f = new Face();
         f->vertices[0] = midP->headV;
         f->vertices[1] = midP->tailV;
         f->vertices[2] = rightP->tailV;
         f->calculateNormal();
         model->faces.push_back(f);
         addFaceToVertexReferences(f);

         f = new Face();
         f->vertices[0] = rightP->headV;
         f->vertices[1] = midP->headV;
         f->vertices[2] = rightP->tailV;
         f->calculateNormal();
         model->faces.push_back(f);
         addFaceToVertexReferences(f);

         // Add in the neighbors
         makeNeighbors(midP->headV, midP->tailV);
         makeNeighbors(midP->headV, rightP->headV);
         makeNeighbors(midP->headV, rightP->tailV);
      }
   }
}

void TerrainGenerator::RemoveRetreatingGeometry() {
   // startTimer();

   // printf("Removing Retreating Geometry =============== count = %d\n", paths.size());
   int numPaths = paths.size();

   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];

      if (p->buildAction == Path::BuildAction::RETREAT) {
         // Merge the head with the tail to delete the headV
         MR::Collapse(model, p->headV, p->tailV);

         // Move the path down
         p->headV = p->tailV;
         p->tailV = neighborFromDirection(p->tailV, - p->heading);
      }
   });
   // printf("Finished retreating =============== count = %d\n", paths.size());
   // printTimeDelta();
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
   // printf("Finished removing converging. Count = %d\n", paths.size());
}

void TerrainGenerator::CalculateVertexNormals() {
   int numPaths = paths.size();
   tbb::parallel_for ((size_t)0, (size_t)numPaths, [&](size_t i) {
      Path * p = paths[i];
      if (p->buildAction == Path::BuildAction::ADVANCE) {
         paths[i]->headV->calculateNormal();
         paths[i]->tailV->calculateNormal();
      }
   });
}
