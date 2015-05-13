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
   void BuildStep();
   PointDist FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist);

private:
   class Path: public GPoint {
   public:
      Vertex *headV, *tailV;
      Path *leftP, *rightP;
      Eigen::Vector3f heading;

      void CalculateHeading();
      Eigen::Vector3f getPosition();
   };

   Model * model;
   SpatialGrid * grid;

   std::vector<Path *> paths;
   int stepCnt;
   float edgeLength;

   void ExtendPaths();
   void SmoothPathPositions();
   void MergePathHeads();
   void AddNeededPaths();
   void AddVerticesAndFaces();
   void RemoveCrossPaths();

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};


#endif // __TERRAIN_H__