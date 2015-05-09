#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "matrix_math.h"
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
   VertDist FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist);

private:
   class Path {
   public:
      Vertex *headV, *tailV;
      Path *leftP, *rightP;
      Eigen::Vector3f heading;

      void CalculateHeading();
   };

   Model * model;
   SpatialGrid * grid;

   std::vector<Path *> paths;
   int stepCnt;
   float edgeLength;

   void ExtendPaths();
   void MergePathHeads();
   void AddNeededPaths();
   void AddVertices();
   void CreateFaces();
   void RemoveCrossPaths();

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};


#endif // __TERRAIN_H__