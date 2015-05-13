#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "matrix_math.h"
#include "geometry.h"
#include "grid.h"

#include <vector>

class Model;
struct Vertex;

class TerrainGenerator {
public:
   TerrainGenerator();
   ~TerrainGenerator();

   Model * GenerateModel();

   // Extends the paths that are within the radius centered at center
   void BuildStep(Eigen::Vector3f center, float radius);
   // Returns the closest point on the mesh to target
   PointDist FindClosestVertex(Eigen::Vector3f target, float maxDist);

private:
   class Path: public GPoint {
   public:
      Vertex *headV, *tailV;
      Path *leftP, *rightP;
      Eigen::Vector3f heading;
      bool extending;

      void CalculateHeading();
      Eigen::Vector3f getPosition();
   };

   Model * model;
   SpatialGrid * grid;

   std::vector<Path *> paths;
   float edgeLength;

   void PickPathsToExtend(Eigen::Vector3f center, float radius);
   void ExtendPaths();
   void SmoothPathPositions();
   void MergePaths();
   void CreateNeededPaths();
   void AddVerticesAndFaces();
   void RemoveConvergingPaths();

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};

#endif // __TERRAIN_H__