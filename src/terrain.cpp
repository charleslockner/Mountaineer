
#include "terrain.h"
#include "stdio.h"
#include "assert.h"

#include <algorithm>

#define MAX_FIND_DIST 40

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

// ============================================================ //
// ===================== PUBLIC FUNCTIONS ===================== //
// ============================================================ //

TerrainGenerator::TerrainGenerator() {};

Model * TerrainGenerator::GenerateModel() {
   edgeLength = 1;

   grid = new SpatialGrid(1000, 10 * edgeLength);

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

   // Add the vertex to the model and grid
   model->vertices.push_back(vStart);
   grid->Add(vStart);

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
      paths[i]->buildAction = Path::BuildAction::ADVANCE;
   }

   ExtendPaths();
   MergePaths();
   CreateNeededPaths();
   AddVerticesAndFaces();
   RemoveConvergingPaths();
   model->CalculateNormals();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->bufferVertices();
   model->bufferIndices();

   return model;
}

void TerrainGenerator::UpdateMesh(Vector3f center, float radius) {

   PickPathsToExtend(center, radius);
   BuildStep();

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();

   model->bufferVertices();
   model->bufferIndices();

   // printf("Step: paths %d, verts %d, faces %d, center (%.2f, %.2f, %.2f)\n", paths.size(), model->vertices.size(), model->faces.size(), center(0), center(1), center(2));
}

PointDist TerrainGenerator::FindClosestToPoint(Eigen::Vector3f targetPnt) {
   return grid->FindClosestToPoint(targetPnt, MAX_FIND_DIST);
}

PointDist TerrainGenerator::FindClosestToLine(Geom::Rayf line) {
   return grid->FindClosestToLine(line, MAX_FIND_DIST);
}

// ============================================================ //
// ===================== PRIVATE FUNCTIONS ==================== //
// ============================================================ //

void TerrainGenerator::BuildStep() {
   ExtendPaths();
   MergePaths();
   CreateNeededPaths();
   AddVerticesAndFaces();
   RemoveConvergingPaths();
   model->CalculateNormals();
}

void TerrainGenerator::PickPathsToExtend(Vector3f center, float radius) {
   int numPaths = paths.size();
   float radiusSq = radius * radius;
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      float headDistSq = (p->headV->position - center).squaredNorm();
      float tailDistSq = (p->tailV->position - center).squaredNorm();

      if (headDistSq < radiusSq)
         p->buildAction = Path::BuildAction::ADVANCE;
      else if (headDistSq > radiusSq && tailDistSq > radiusSq)
         p->buildAction = Path::BuildAction::RETREAT;
      else
         p->buildAction = Path::BuildAction::STATION;
   }
}

void TerrainGenerator::ExtendPaths() {
   int numPaths = paths.size();

   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];

      if (p->buildAction == Path::BuildAction::ADVANCE) {
         Vector3f randY = randRange(-0.05, 0.05) * p->headV->normal;

         // Add vertex created from extending the path
         Vertex * v = new Vertex();
         v->position = p->headV->position + edgeLength * (p->heading + randY).normalized();
         v->tangent = p->headV->tangent;
         v->bitangent = p->headV->bitangent;
         v->normal = p->headV->normal;

         // Update the path
         p->tailV = p->headV;
         p->headV = v;

         p->calculateHeading();
      }
   }

   // Smooth the positions of the newly created vertices
   std::vector<Vector3f> updatedPositions = std::vector<Vector3f>(numPaths);
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];

      if (p->buildAction == Path::BuildAction::ADVANCE) {
         Vector3f midTail = 0.5f * (p->leftP->tailV->position + p->rightP->tailV->position);
         Vector3f midHead = 0.5f * (p->leftP->headV->position + p->rightP->headV->position);
         Vector3f midDir = directionFromPoints(midHead, midTail);
         Vector3f properPos = midTail + edgeLength * midDir;
         updatedPositions[i] = 0.25f * p->headV->position + 0.75f * properPos;
      }
   }

   // Update the position and uv coords
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      if (paths[i]->buildAction == Path::BuildAction::ADVANCE)
         p->headV->position = updatedPositions[i];
      p->calculateUV();
   }
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
         grid->Add(selfP->headV);
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
   if (leftP->buildAction == Path::BuildAction::ADVANCE &&
       rightP->buildAction == Path::BuildAction::ADVANCE) {
      // Create the emerging face
      Face * f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->tailV;
      model->faces.push_back(f);
   }
}

void TerrainGenerator::HandleSameTail(Path * leftP, Path * rightP) {
   if (leftP->buildAction == Path::BuildAction::ADVANCE) {
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
   if (leftP->buildAction == Path::BuildAction::ADVANCE &&
       rightP->buildAction != Path::BuildAction::ADVANCE) {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = leftP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);
   } else if (leftP->buildAction != Path::BuildAction::ADVANCE &&
              rightP->buildAction == Path::BuildAction::ADVANCE) {
      f = new Face();
      f->vertices[0] = leftP->headV;
      f->vertices[1] = rightP->tailV;
      f->vertices[2] = rightP->headV;
      model->faces.push_back(f);
   } else if (leftP->buildAction == Path::BuildAction::ADVANCE &&
              rightP->buildAction == Path::BuildAction::ADVANCE) {
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
