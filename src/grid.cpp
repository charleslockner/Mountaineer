#include "grid.h"
#include "assert.h"

// ================================================================== //
// ======================== STATIC FUNCTIONS ======================== //
// ================================================================== //

static inline float square(float n) {
   return n*n;
}

// ================================================================== //
// ========================= CELL FUNCTIONS ========================= //
// ================================================================== //

SpatialGrid::Cell::Cell() {
   _points = std::vector<Geom::Positionalf *>(0);
}
SpatialGrid::Cell::~Cell() {}

void SpatialGrid::Cell::Add(Geom::Positionalf * pnt) {
   _points.push_back(pnt);
}

PointDist SpatialGrid::Cell::FindClosest(Eigen::Vector3f targetPnt) {
   PointDist shortest;
   shortest.distSq = -1;
   shortest.pnt = NULL;

   if (_points.size() == 0)
      return shortest;
   else {
      shortest.distSq = (targetPnt - _points[0]->getPosition()).squaredNorm();
      shortest.pnt = _points[0];

      for (int i = 1; i < _points.size(); i++) {
         float distSq = (targetPnt - _points[i]->getPosition()).squaredNorm();
         if (distSq < shortest.distSq) {
            shortest.distSq = distSq;
            shortest.pnt = _points[i];
         }
      }

      return shortest;
   }
}

// ================================================================== //
// ======================== PUBLIC FUNCTIONS ======================== //
// ================================================================== //

SpatialGrid::SpatialGrid(int maxAcross, float cellWidth)
: _maxAcross(maxAcross), _cellWidth(cellWidth) {

   _cells = std::deque<std::deque<std::deque<Cell> > >(MIN_ACROSS);
   for (int i = 0; i < MIN_ACROSS; i++) {
      _cells[i] = std::deque<std::deque<Cell> >(MIN_ACROSS);
      for (int j = 0; j < MIN_ACROSS; j++) {
         _cells[i][j] = std::deque<Cell>(MIN_ACROSS);
         for (int k = 0; k < MIN_ACROSS; k++)
            _cells[i][j][k] = Cell();
      }
   }

   _xIndexOffset = _yIndexOffset = _zIndexOffset = 0;
   _pointCount = 0;
}

SpatialGrid::~SpatialGrid() {}

uint SpatialGrid::xSize() {
   return _cells.size();
}
uint SpatialGrid::ySize() {
   return _cells[0].size();
}
uint SpatialGrid::zSize() {
   return _cells[0][0].size();
}

void SpatialGrid::Add(Geom::Positionalf * pnt) {
   Eigen::Vector3i indexV = FindIndicesFromPoint(pnt->getPosition());
   indexV = expandIfNeeded(indexV);
   Cell * cell = GetCellAt(indexV);
   cell->Add(pnt);
   assert(cell); // Since we resized to fit the cell, it shouldn't be null;
   _pointCount++;
}

PointDist SpatialGrid::FindClosestToPoint(Eigen::Vector3f target) {
   Eigen::Vector3i indexV = FindIndicesFromPoint(target);
   indexV = expandIfNeeded(indexV);
   Cell * cell = GetCellAt(indexV);
   assert(cell); // Since we resized to fit the cell, it shouldn't be null;

   PointDist closestPD = cell->FindClosest(target);

   // If there is a vertex within the same cell as the target
   if (closestPD.pnt) {
      // Find all neighboring cells that need to be checked against the found point
      std::vector<Cell *> checkCells = findCheckCells(target, closestPD.distSq);

      // Compare distances from each cell to find the shortest to the target
      for (int i = 0; i < checkCells.size(); i++) {
         PointDist iPointDist = checkCells[i]->FindClosest(target);
         if (iPointDist.pnt && iPointDist.distSq < closestPD.distSq)
            closestPD = iPointDist;
      }

      return closestPD;
   }
   // If there are no points within the same cell as the target
   else {
      return closestPD;
      // return FindClosestToPointFromNeighbors(target, 1);
   }
}

// ================================================================== //
// ======================== PRIVATE FUNCTIONS ======================= //
// ================================================================== //


PointDist SpatialGrid::FindClosestToPointFromNeighbors(Eigen::Vector3f target, int cellsOut) {
   PointDist closestPD;
   closestPD.pnt = NULL;
   closestPD.distSq = -1;

   if (_pointCount <= 0 || cellsOut >= std::max(std::max(xSize(), ySize()), zSize())) {
      // printf("Too far out at %d\n", cellsOut);
      return closestPD;
   }
   else {
      Eigen::Vector3i centerIndexV = FindIndicesFromPoint(target);
      std::vector<Cell *> checkCells = findNeighborCells(centerIndexV, cellsOut);
      // printf("checkCells size = %d\n", checkCells.size());

      // Compare distances from each cell to find the shortest to the target
      for (int i = 0; i < checkCells.size(); i++) {
         PointDist iPD = checkCells[i]->FindClosest(target);

         if (closestPD.pnt)
            if (iPD.pnt && iPD.distSq < closestPD.distSq) {
               // printf("I found it!! %d\n", i);
               closestPD = iPD;
            }
         else if (iPD.pnt) {
            // printf("There is one! %d\n", i);
            closestPD = iPD;
         }
      }

      if (closestPD.pnt) {
         // printf("got it %p\n", closestPD.pnt);
         return closestPD;
      }
      else {
         // printf("can't find it\n");
         return FindClosestToPointFromNeighbors(target, cellsOut + 1);
      }
   }
}

std::vector<SpatialGrid::Cell *> SpatialGrid::findNeighborCells(Eigen::Vector3i centerIndexV, int cellsOut) {
   std::vector<Cell *> checkCells = std::vector<Cell *>(0);
   int sideLen = 2 * cellsOut + 1;
   Cell * cell;

   // printf("findNeighborcells center %d %d %d\n", centerIndexV(0), centerIndexV(1), centerIndexV(2));

   // Add the left and right sides
   // printf("sizes %d %d %d\n", xSize(), ySize(), zSize());
   // printf("-sidelen/2 = %d\n", -sideLen/2);
   // printf("sidelen/2 = %d\n", sideLen/2);
   for (int j = -sideLen/2; j <= sideLen/2; j++) {
      // printf("j = %d\n", j);
      for (int k = -sideLen/2; k <= sideLen/2; k++) {
         // printf("k = %d\n", k);
         // printf("index %d %d %d\n", (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(0), (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(1), (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(2));
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(-cellsOut, j, k));
         if (cell)
            checkCells.push_back(cell);
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(cellsOut, j, k));
         if (cell)
            checkCells.push_back(cell);
         // if (cell)
         //    printf("there's a cell here! %d %d\n", j, k);
      }
   }
   // Add the top and bottom sides
   for (int i = -sideLen/2+1; i <= sideLen/2-1; i++) {
      for (int k = -sideLen/2; k < sideLen/2; k++) {
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, -cellsOut, k));
         if (cell)
            checkCells.push_back(cell);
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, cellsOut, k));
         if (cell)
            checkCells.push_back(cell);
      }
   }
   // Add the front and back sides
   for (int i = -sideLen/2+1; i <= sideLen/2-1; i++) {
      for (int j = -sideLen/2+1; j < sideLen/2-1; j++) {
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, j, -cellsOut));
         if (cell)
            checkCells.push_back(cell);
         cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, j, cellsOut));
         if (cell)
            checkCells.push_back(cell);
      }
   }

   return checkCells;
}

std::vector<SpatialGrid::Cell *> SpatialGrid::findCheckCells(Eigen::Vector3f target, float foundDistSq) {
   Eigen::Vector3i indexV = FindIndicesFromPoint(target);
   Eigen::Vector3i worldIndexV = GetVirtualIndexFromReal(indexV);

   // Calculate the grid boundaries
   Eigen::Vector3f gridPositionNegV = _cellWidth * worldIndexV.cast<float>();
   Eigen::Vector3f gridPositionPosV = _cellWidth * (worldIndexV + Eigen::Vector3i(1,1,1)).cast<float>();
   Eigen::Vector3f distToGridNegV = target - gridPositionNegV;
   Eigen::Vector3f distToGridPosV = gridPositionPosV - target;

   // Check to see if any other neighboring cells might contain a closer point
   // We only have to consider cells who's boundaries are closer than the distance
   // to the point within the current cell
   bool checkGridNegX = square(distToGridNegV(0)) < foundDistSq;
   bool checkGridPosX = square(distToGridPosV(0)) < foundDistSq;
   bool checkGridNegY = square(distToGridNegV(1)) < foundDistSq;
   bool checkGridPosY = square(distToGridPosV(1)) < foundDistSq;
   bool checkGridNegZ = square(distToGridNegV(2)) < foundDistSq;
   bool checkGridPosZ = square(distToGridPosV(2)) < foundDistSq;

   // Start off needing to check all surrounding cells
   int needToCheck[3][3][3];
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         for (int k = 0; k < 3; k++)
            needToCheck[i][j][k] = true;
   // Cross off sections based on checkGrid booleans
   for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++)
         if (!checkGridNegX)
            needToCheck[0][j][k] = false;
   for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++)
         if (!checkGridPosX)
            needToCheck[2][j][k] = false;
   for (int i = 0; i < 3; i++)
      for (int k = 0; k < 3; k++)
         if (!checkGridNegY)
            needToCheck[i][0][k] = false;
   for (int i = 0; i < 3; i++)
      for (int k = 0; k < 3; k++)
         if (!checkGridPosY)
            needToCheck[i][2][k] = false;
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         if (!checkGridNegZ)
            needToCheck[i][j][0] = false;
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         if (!checkGridPosZ)
            needToCheck[i][j][2] = false;
   // We already have a value for the center, so we don't need to check this
   needToCheck[1][1][1] = false;

   std::vector<Cell *> checkCells = std::vector<Cell *>(0);
   checkCells.reserve(26);

   // Add all the cells we need to check to a vector
   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
         for (int k = 0; k < 3; k++) {
            if (needToCheck[i][j][k]) {
               Cell * cell = GetCellAt(indexV + Eigen::Vector3i(i-1,j-1,k-1));
               if (cell)
                  checkCells.push_back(cell);
            }
         }
      }
   }

   return checkCells;
}

Eigen::Vector3i SpatialGrid::GetVirtualIndexFromReal(Eigen::Vector3i indexV) {
   return indexV - Eigen::Vector3i(_xIndexOffset, _yIndexOffset, _zIndexOffset);
}

Eigen::Vector3i SpatialGrid::FindIndicesFromPoint(Eigen::Vector3f pnt) {
   int xIndex = int(pnt(0) / _cellWidth) + (pnt(0) < 0 ? -1 : 0) + _xIndexOffset;
   int yIndex = int(pnt(1) / _cellWidth) + (pnt(1) < 0 ? -1 : 0) + _yIndexOffset;
   int zIndex = int(pnt(2) / _cellWidth) + (pnt(2) < 0 ? -1 : 0) + _zIndexOffset;
   return Eigen::Vector3i(xIndex, yIndex, zIndex);
}

SpatialGrid::Cell * SpatialGrid::GetCellAt(Eigen::Vector3i indexV) {
   if ( indexV(0) < 0 || indexV(0) >= xSize() ||
        indexV(1) < 0 || indexV(1) >= ySize() ||
        indexV(2) < 0 || indexV(2) >= zSize() )
      return NULL;
   else
      return & _cells[indexV(0)][indexV(1)][indexV(2)];
}

Eigen::Vector3i SpatialGrid::expandIfNeeded(Eigen::Vector3i indexV) {
   int xIndex = indexV(0);
   int yIndex = indexV(1);
   int zIndex = indexV(2);

   if (xIndex < 0) {
      uint addSpace = -xIndex + ADD_RESIZE_SPACE;
      AddSpaceNegX(addSpace);
      _xIndexOffset += addSpace;
      xIndex += addSpace;
   }
   else if (xIndex >= xSize()) {
      AddSpacePosX(xIndex - xSize() + ADD_RESIZE_SPACE);
   }

   if (yIndex < 0) {
      uint addSpace = -yIndex + ADD_RESIZE_SPACE;
      AddSpaceNegY(addSpace);
      _yIndexOffset += addSpace;
      yIndex += addSpace;
   }
   else if (yIndex >= ySize()) {
      AddSpacePosY(yIndex - ySize() + ADD_RESIZE_SPACE);
   }

   if (zIndex < 0) {
      uint addSpace = -zIndex + ADD_RESIZE_SPACE;
      AddSpaceNegZ(addSpace);
      _zIndexOffset += addSpace;
      zIndex += addSpace;
   }
   else if (zIndex >= zSize()) {
      AddSpacePosZ(zIndex - zSize() + ADD_RESIZE_SPACE);
   }

   return Eigen::Vector3i(xIndex, yIndex, zIndex);
}

void SpatialGrid::AddSpaceNegX(uint numSlots) {
   std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
   for (int j = 0; j < ySize(); j++) {
      yzContainers[j] = std::deque<Cell>(zSize());
      for (int k = 0; k < zSize(); k++)
         yzContainers[j][k] = Cell();
   }

   _cells.insert(_cells.begin(), numSlots, yzContainers);
}
void SpatialGrid::AddSpacePosX(uint numSlots) {
   std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
   for (int j = 0; j < ySize(); j++) {
      yzContainers[j] = std::deque<Cell>(zSize());
      for (int k = 0; k < zSize(); k++)
         yzContainers[j][k] = Cell();
   }

   _cells.insert(_cells.end(), numSlots, yzContainers);
}
void SpatialGrid::AddSpaceNegY(uint numSlots) {
   std::deque<Cell> zContainers = std::deque<Cell>(zSize());
   for (int k = 0; k < zSize(); k++)
      zContainers[k] = Cell();

   for (int i = 0; i < xSize(); i++)
      _cells[i].insert(_cells[i].begin(), numSlots, zContainers);
}
void SpatialGrid::AddSpacePosY(uint numSlots) {
   std::deque<Cell> zContainers = std::deque<Cell>(zSize());
   for (int k = 0; k < zSize(); k++)
      zContainers[k] = Cell();

   for (int i = 0; i < xSize(); i++)
      _cells[i].insert(_cells[i].end(), numSlots, zContainers);
}
void SpatialGrid::AddSpaceNegZ(uint numSlots) {
   Cell cellContainer = Cell();
   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         _cells[i][j].insert(_cells[i][j].begin(), numSlots, cellContainer);
}
void SpatialGrid::AddSpacePosZ(uint numSlots) {
   Cell cellContainer = Cell();
   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         _cells[i][j].insert(_cells[i][j].end(), numSlots, cellContainer);
}

void SpatialGrid::RemoveFromNegX(uint numSlots) {
   int sizeMinDiff = xSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   _cells.erase(_cells.begin(), _cells.begin() + depth);
}
void SpatialGrid::RemoveFromPosX(uint numSlots) {
   int sizeMinDiff = xSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   _cells.erase(_cells.end() - depth, _cells.end());
}
void SpatialGrid::RemoveFromNegY(uint numSlots) {
   int sizeMinDiff = ySize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      _cells[i].erase(_cells[i].begin(), _cells[i].begin() + depth);
}
void SpatialGrid::RemoveFromPosY(uint numSlots) {
   int sizeMinDiff = ySize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      _cells[i].erase(_cells[i].end() - depth, _cells[i].end());
}
void SpatialGrid::RemoveFromNegZ(uint numSlots) {
   int sizeMinDiff = zSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         _cells[i][j].erase(_cells[i][j].begin(), _cells[i][j].begin() + depth);
}
void SpatialGrid::RemoveFromPosZ(uint numSlots) {
   int sizeMinDiff = zSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         _cells[i][j].erase(_cells[i][j].end() - depth, _cells[i][j].end());
}
