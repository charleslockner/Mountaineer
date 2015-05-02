#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "model.h"

class TerrainGenerator {
public:
   TerrainGenerator();
   ~TerrainGenerator();

   Model * generateRockFace();
};


#endif // __TERRAIN_H__