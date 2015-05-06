#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "model.h"
#include "matrix_math.h"
#include "grid.h"

class TerrainGenerator {
public:
   TerrainGenerator();
   ~TerrainGenerator();

   Model * GenerateModel();
   void BuildStep();
   VertDist FindClosestVertex(Eigen::Vector3f targetPnt, float maxDist);

private:
   typedef struct Path {
      Vertex *headV, *tailV;
      Path *leftP, *rightP;
   } Path;

   Model * model;
   SpatialGrid * grid;

   std::vector<Path *> paths;
   int stepCnt;
   float edgeLength;


   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleDiffTail(Path * leftP, Path * rightP);
   void HandleOKHeadDist(Path * leftP, Path * rightP);
   void HandleBigHeadDist(Path * leftP, Path * rightP);
};


#endif // __TERRAIN_H__