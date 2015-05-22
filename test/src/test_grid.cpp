#include "test.h"

#include "grid.h"

using namespace Eigen;
using namespace Geom;

void testGrid() {
   // Check grid's add and remove methodd
   {
      SpatialGrid grid = SpatialGrid(10, 1);
      equalityIntCheck(grid.xSize(), 1);
      equalityIntCheck(grid.ySize(), 1);
      equalityIntCheck(grid.zSize(), 1);
      equalityIntCheck(grid.minXIndex(), 0);
      equalityIntCheck(grid.maxXIndex(), 0);
      equalityIntCheck(grid.minYIndex(), 0);
      equalityIntCheck(grid.maxYIndex(), 0);
      equalityIntCheck(grid.minZIndex(), 0);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.numElements(), 0);

      Pointf * p1 = new Pointf(Vector3f(5.5,0.5,0.5));
      grid.Add(p1);
      equalityIntCheck(grid.xSize(), 6);
      equalityIntCheck(grid.ySize(), 1);
      equalityIntCheck(grid.zSize(), 1);
      equalityIntCheck(grid.minXIndex(), 0);
      equalityIntCheck(grid.maxXIndex(), 5);
      equalityIntCheck(grid.minYIndex(), 0);
      equalityIntCheck(grid.maxYIndex(), 0);
      equalityIntCheck(grid.minZIndex(), 0);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.numElements(), 1);

      Pointf * p2 = new Pointf(Vector3f(0.5,-10.5,0.5));
      grid.Add(p2);
      equalityIntCheck(grid.xSize(), 6);
      equalityIntCheck(grid.ySize(), 12);
      equalityIntCheck(grid.zSize(), 1);
      equalityIntCheck(grid.minXIndex(), 0);
      equalityIntCheck(grid.maxXIndex(), 5);
      equalityIntCheck(grid.minYIndex(), -11);
      equalityIntCheck(grid.maxYIndex(), 0);
      equalityIntCheck(grid.minZIndex(), 0);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.numElements(), 2);

      Pointf * p3 = new Pointf(Vector3f(4.5,-10.5,-200.5));
      grid.Add(p3);
      equalityIntCheck(grid.xSize(), 6);
      equalityIntCheck(grid.ySize(), 12);
      equalityIntCheck(grid.zSize(), 202);
      equalityIntCheck(grid.minXIndex(), 0);
      equalityIntCheck(grid.maxXIndex(), 5);
      equalityIntCheck(grid.minYIndex(), -11);
      equalityIntCheck(grid.maxYIndex(), 0);
      equalityIntCheck(grid.minZIndex(), -201);
      equalityIntCheck(grid.maxZIndex(), 0);
      equalityIntCheck(grid.numElements(), 3);
   }

   // Check grid's
}