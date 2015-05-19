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
   Geom::Positionalf * pnt;
   float distSq;
} PointDist;

class SpatialGrid {
public:
   SpatialGrid(int maxAcross, float cellWidth);
   ~SpatialGrid();

   uint xSize();
   uint ySize();
   uint zSize();

   void Add(Geom::Positionalf * pnt);
   PointDist FindClosestToPoint(Eigen::Vector3f target);
   PointDist FindClosestToLine(Geom::Linef line);

private:
   class Cell {
   public:
      Cell();
      ~Cell();

      void Add(Geom::Positionalf * pnt);
      PointDist FindClosest(Eigen::Vector3f target);

   private:
      std::vector<Geom::Positionalf *> _points;
   };

   uint _pointCount;
   uint _maxAcross;
   float _cellWidth;
   int _xIndexOffset, _yIndexOffset, _zIndexOffset;
   std::deque<std::deque<std::deque<Cell> > > _cells;


   // Find all neighboring cells that need to be checked against the found point
   std::vector<Cell *> findCheckCells(Eigen::Vector3f target, float foundDistSq);
   // Recursively searches surrounding cells for the closest point
   PointDist FindClosestToPointFromNeighbors(Eigen::Vector3f target, int cellsOut);
   // Returns a list of all the cells that are "cellsOut" away from the index
   std::vector<SpatialGrid::Cell *> findNeighborCells(Eigen::Vector3i centerIndexV, int cellsOut);

   // Get the index as seen in the world, using the xyz offset indices
   Eigen::Vector3i GetVirtualIndexFromReal(Eigen::Vector3i indexV);
   // Returns the indices even if they are beyond the bounds of the grid
   Eigen::Vector3i FindIndicesFromPoint(Eigen::Vector3f pnt);
   // Returns NULL, if pnt is not within any existing cell
   Cell * GetCellAt(Eigen::Vector3i indexV);

   Eigen::Vector3i expandIfNeeded(Eigen::Vector3i indexV);
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