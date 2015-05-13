#ifndef __GRID_H__
#define __GRID_H__

#include <vector>
#include <deque>

#include "model.h"
#include "matrix_math.h"
#include "geometry.h"

#define MIN_ACROSS 1
#define ADD_RESIZE_SPACE 10

typedef unsigned int uint;

typedef struct {
   GPoint * pnt;
   float distSq;
} PointDist;

class SpatialGrid {
public:
   SpatialGrid(int maxAcross, float cellWidth);
   ~SpatialGrid();

   uint xSize();
   uint ySize();
   uint zSize();
   void Add(GPoint * pnt);
   PointDist FindClosest(Eigen::Vector3f target, float maxDist);

private:
   class Cell {
   public:
      Cell();
      ~Cell();

      void Add(GPoint * pnt);
      PointDist FindClosest(Eigen::Vector3f target, float maxDist);

   private:
      std::vector<GPoint *> points;
   };

   uint maxAcross;
   float cellWidth;
   int xIndexOffset, yIndexOffset, zIndexOffset;
   std::deque<std::deque<std::deque<Cell> > > cells;

   // Get the index as seen in the world, using the xyz offset indices
   Eigen::Vector3i GetVirtualIndexFromReal(Eigen::Vector3i indexV);
   // Returns the indices even if they are beyond the bounds of the grid
   Eigen::Vector3i FindIndicesFromPoint(Eigen::Vector3f pnt);
   // Returns NULL, if pnt is not within any existing cell
   Cell * GetCellAt(Eigen::Vector3i indexV);

   void AddSpaceNegX(uint numSlots);
   void AddSpacePosX(uint numSlots);
   void AddSpaceNegY(uint numSlots);
   void AddSpacePosY(uint numSlots);
   void AddSpaceNegZ(uint numSlots);
   void AddSpacePosZ(uint numSlots);

   void RemoveFromNegX(uint numSlots);
   void RemoveFromPosX(uint numSlots);
   void RemoveFromNegY(uint numSlots);
   void RemoveFromPosY(uint numSlots);
   void RemoveFromNegZ(uint numSlots);
   void RemoveFromPosZ(uint numSlots);
};

#endif // __GRID_H__