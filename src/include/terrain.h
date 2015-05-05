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
   typedef struct {
      Vertex *head, *tail, *left, *right;
      Eigen::Vector3f direction;
   } Path;

   Model * model;
   std::vector<Path> paths;
};


#endif // __TERRAIN_H__