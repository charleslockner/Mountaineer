#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "model.h"
#include "matrix_math.h"

class TerrainGenerator {
public:
   TerrainGenerator();
   ~TerrainGenerator();

   Model * GenerateModel();
   void BuildStep();

private:
   typedef struct Path {
      Vertex *headV, *tailV;
      Path *leftP, *rightP;
   } Path;

   Model * model;
   std::vector<Path *> paths;
   int stepCnt;
   float edgeLength;
};


#endif // __TERRAIN_H__