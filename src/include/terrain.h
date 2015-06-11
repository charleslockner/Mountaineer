#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "matrix_math.h"
#include "geometry.h"
#include "grid.h"
#include "model.h"

#include <vector>

#define UV_STEP_SIZE .025

class TerrainGenerator {
public:
   TerrainGenerator();
   // Creates and returns the initial model
   Model * GenerateModel();
   // Extends the paths that are within the sphere, and removes the paths that are outside of it
   void UpdateMesh(Eigen::Vector3f center, float radius);

   class Path {
   public:
      enum BuildAction {
         ADVANCE, // path is about to create some geometry
         RETREAT, // path is about to remove some geometry
         STATION  // path is neither creating nor removing geometry
      };

      Vertex *headV, *tailV;
      Path *leftP, *rightP;
      Eigen::Vector3f heading;
      BuildAction buildAction;

      inline void calculateHeading() {
         heading = (headV->position - tailV->position).normalized();
      }
      inline void calculateUV() {
         Eigen::Matrix3f iTBN = Mmath::InverseTBN(headV->tangent, headV->bitangent, headV->normal);
         headV->uv = tailV->uv + UV_STEP_SIZE * (iTBN * (headV->position - tailV->position)).head<2>();
      }
   };

   std::vector<Path *> paths;
   Model * model;

private:
   bool shouldUpdate;
   float edgeLength;

   void BuildStep();
   void PickPathsToExtend(Eigen::Vector3f center, float radius);

   void ExtendPaths();
   void MergePaths();
   void CreateNeededPaths();
   void AddVerticesAndFaces();
   void RemoveRetreatingGeometry();
   void RemoveConvergingPaths();
   void CalculateVertexNormals();
   void collapsePath(Path * p);

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};

#endif // __TERRAIN_H__