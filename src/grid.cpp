#include "grid.h"

// --------------------- Cell Stuff --------------------- //
SpatialGrid::Cell::Cell() {
   points = std::vector<GPoint *>(0);
}
SpatialGrid::Cell::~Cell() {}

void SpatialGrid::Cell::Add(GPoint * pnt) {
   points.push_back(pnt);
}

PointDist SpatialGrid::Cell::FindClosest(Eigen::Vector3f targetPnt, float maxDist) {
   PointDist shortest;
   shortest.pnt = NULL;

   if (points.size() == 0)
      return shortest;

   float shortestDistSq = (targetPnt - points[0]->getPosition()).squaredNorm();
   int shortestIndex = 0;
   for (int i = 1; i < points.size(); i++) {
      float distSq = (targetPnt - points[i]->getPosition()).squaredNorm();
      if (distSq < shortestDistSq) {
         shortestDistSq = distSq;
         shortestIndex = i;
      }
   }

   if (shortestDistSq < maxDist * maxDist)
      shortest.pnt = points[shortestIndex];
   shortest.distSq = shortestDistSq;

   return shortest;
}

// ----------------- Spatial Grid Stuff ----------------- //
SpatialGrid::SpatialGrid(int maxAcross, float cellWidth)
: maxAcross(maxAcross), cellWidth(cellWidth) {

   cells = std::deque<std::deque<std::deque<Cell> > >(MIN_ACROSS);
   for (int i = 0; i < MIN_ACROSS; i++) {
      cells[i] = std::deque<std::deque<Cell> >(MIN_ACROSS);
      for (int j = 0; j < MIN_ACROSS; j++) {
         cells[i][j] = std::deque<Cell>(MIN_ACROSS);
         for (int k = 0; k < MIN_ACROSS; k++)
            cells[i][j][k] = Cell();
      }
   }

   xIndexOffset = yIndexOffset = zIndexOffset = 0;
}

SpatialGrid::~SpatialGrid() {}

uint SpatialGrid::xSize() {
   return cells.size();
}
uint SpatialGrid::ySize() {
   return cells[0].size();
}
uint SpatialGrid::zSize() {
   return cells[0][0].size();
}

void SpatialGrid::Add(GPoint * pnt) {
   Eigen::Vector3i indexV = FindIndicesFromPoint(pnt->getPosition());
   int xIndex = indexV(0);
   int yIndex = indexV(1);
   int zIndex = indexV(2);

   if (xIndex < 0) {
      uint addSpace = -xIndex + ADD_RESIZE_SPACE;
      printf("add space = %d\n", addSpace);
      AddSpaceNegX(addSpace);
      xIndexOffset += addSpace;
      xIndex += addSpace;
   }
   else if (xIndex >= xSize())
      AddSpacePosX(xIndex - xSize() + ADD_RESIZE_SPACE);

   if (yIndex < 0) {
      uint addSpace = -yIndex + ADD_RESIZE_SPACE;
      AddSpaceNegY(addSpace);
      yIndexOffset += addSpace;
      yIndex += addSpace;
   }
   else if (yIndex >= ySize())
      AddSpacePosY(yIndex - ySize() + ADD_RESIZE_SPACE);

   if (zIndex < 0) {
      uint addSpace = -zIndex + ADD_RESIZE_SPACE;
      AddSpaceNegZ(addSpace);
      zIndexOffset += addSpace;
      zIndex += addSpace;
   }
   else if (zIndex >= zSize())
      AddSpacePosZ(zIndex - zSize() + ADD_RESIZE_SPACE);

   cells[xIndex][yIndex][zIndex].Add(pnt);
}

PointDist SpatialGrid::FindClosest(Eigen::Vector3f target, float maxDist) {
   Eigen::Vector3i indexV = FindIndicesFromPoint(target);
   Eigen::Vector3i worldIndexV = GetVirtualIndexFromReal(indexV);
   Cell * cell = GetCellAt(indexV);

   PointDist shortestPointDist;
   shortestPointDist.pnt = NULL;

   if (!cell)
      return shortestPointDist;

   shortestPointDist = cell->FindClosest(target, maxDist);

   // If there is a vertex within the same cell as the target
   if (shortestPointDist.pnt) {
      float foundDistSq = shortestPointDist.distSq;

      // Calculate the grid boundaries
      Eigen::Vector3f gridPositionNegV = cellWidth * worldIndexV.cast<float>();
      Eigen::Vector3f gridPositionPosV = cellWidth * (worldIndexV + Eigen::Vector3i(1,1,1)).cast<float>();
      Eigen::Vector3f distToGridNegV = target - gridPositionNegV;
      Eigen::Vector3f distToGridPosV = gridPositionPosV - target;

      // Check to see if any other neighboring cells might contain a closer point
      // Only have to consider cells who's boundaries are closer than the distance
      //  to the point within the current cell
      bool checkGridNegX = distToGridNegV(0) * distToGridNegV(0) < foundDistSq;
      bool checkGridPosX = distToGridPosV(0) * distToGridPosV(0) < foundDistSq;
      bool checkGridNegY = distToGridNegV(1) * distToGridNegV(1) < foundDistSq;
      bool checkGridPosY = distToGridPosV(1) * distToGridPosV(1) < foundDistSq;
      bool checkGridNegZ = distToGridNegV(2) * distToGridNegV(2) < foundDistSq;
      bool checkGridPosZ = distToGridPosV(2) * distToGridPosV(2) < foundDistSq;

      // Add all the posible grid locations to the grid index vector
      std::vector<Eigen::Vector3i> gridIndices = std::vector<Eigen::Vector3i>(0);

      // Check all 8 corners
      if (checkGridNegX && checkGridNegY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1,-1,-1));
      if (checkGridNegX && checkGridNegY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1,-1, 1));
      if (checkGridNegX && checkGridPosY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 1,-1));
      if (checkGridNegX && checkGridPosY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 1, 1));

      if (checkGridPosX && checkGridNegY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1,-1,-1));
      if (checkGridPosX && checkGridNegY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1,-1, 1));
      if (checkGridPosX && checkGridPosY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 1,-1));
      if (checkGridPosX && checkGridPosY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 1, 1));

      // Check all 12 middle edges
      if (checkGridNegX && checkGridNegY)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1,-1, 0));
      if (checkGridNegX && checkGridPosY)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 1, 0));
      if (checkGridNegX && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 0,-1));
      if (checkGridNegX && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 0, 1));

      if (checkGridNegY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0,-1,-1));
      if (checkGridNegY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0,-1, 1));
      if (checkGridPosY && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0, 1,-1));
      if (checkGridPosY && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0, 1, 1));

      if (checkGridPosX && checkGridNegY)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1,-1, 0));
      if (checkGridPosX && checkGridPosY)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 1, 0));
      if (checkGridPosX && checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 0,-1));
      if (checkGridPosX && checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 0, 1));

      // Check all 6 centers
      if (checkGridNegX)
         gridIndices.push_back(indexV + Eigen::Vector3i(-1, 0, 0));
      if (checkGridPosX)
         gridIndices.push_back(indexV + Eigen::Vector3i( 1, 0, 0));
      if (checkGridNegY)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0,-1, 0));
      if (checkGridPosY)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0, 1, 0));
      if (checkGridNegZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0, 0,-1));
      if (checkGridPosZ)
         gridIndices.push_back(indexV + Eigen::Vector3i( 0, 0, 1));

      // Compare distances from each cell to find the shortest to the target
      for (int i = 0; i < gridIndices.size(); i++) {
         Cell * iCell = GetCellAt(gridIndices[i]);
         if (iCell) {
            PointDist iPointDist = iCell->FindClosest(target, maxDist);
            if (iPointDist.pnt && iPointDist.distSq < shortestPointDist.distSq)
               shortestPointDist = iPointDist;
         }
      }

      // Return the closest vertex
      return shortestPointDist;
   }
   else {
      return shortestPointDist; // Figure this out later
   }
}

Eigen::Vector3i SpatialGrid::GetVirtualIndexFromReal(Eigen::Vector3i indexV) {
   return indexV - Eigen::Vector3i(xIndexOffset, yIndexOffset, zIndexOffset);
}

Eigen::Vector3i SpatialGrid::FindIndicesFromPoint(Eigen::Vector3f pnt) {
   int xIndex = int(pnt(0) / cellWidth) + (pnt(0) < 0 ? -1 : 0) + xIndexOffset;
   int yIndex = int(pnt(1) / cellWidth) + (pnt(1) < 0 ? -1 : 0) + yIndexOffset;
   int zIndex = int(pnt(2) / cellWidth) + (pnt(2) < 0 ? -1 : 0) + zIndexOffset;
   return Eigen::Vector3i(xIndex, yIndex, zIndex);
}

SpatialGrid::Cell * SpatialGrid::GetCellAt(Eigen::Vector3i indexV) {
   if ( indexV(0) < 0 || indexV(0) >= xSize() ||
        indexV(1) < 0 || indexV(1) >= ySize() ||
        indexV(2) < 0 || indexV(2) >= zSize() )
      return NULL;
   else
      return & cells[indexV(0)][indexV(1)][indexV(2)];
}

void SpatialGrid::AddSpaceNegX(uint numSlots) {
   std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
   for (int j = 0; j < ySize(); j++) {
      yzContainers[j] = std::deque<Cell>(zSize());
      for (int k = 0; k < zSize(); k++)
         yzContainers[j][k] = Cell();
   }

   cells.insert(cells.begin(), numSlots, yzContainers);
}
void SpatialGrid::AddSpacePosX(uint numSlots) {
   std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
   for (int j = 0; j < ySize(); j++) {
      yzContainers[j] = std::deque<Cell>(zSize());
      for (int k = 0; k < zSize(); k++)
         yzContainers[j][k] = Cell();
   }

   cells.insert(cells.end(), numSlots, yzContainers);
}
void SpatialGrid::AddSpaceNegY(uint numSlots) {
   std::deque<Cell> zContainers = std::deque<Cell>(zSize());
   for (int k = 0; k < zSize(); k++)
      zContainers[k] = Cell();

   for (int i = 0; i < xSize(); i++)
      cells[i].insert(cells[i].begin(), numSlots, zContainers);
}
void SpatialGrid::AddSpacePosY(uint numSlots) {
   std::deque<Cell> zContainers = std::deque<Cell>(zSize());
   for (int k = 0; k < zSize(); k++)
      zContainers[k] = Cell();

   for (int i = 0; i < xSize(); i++)
      cells[i].insert(cells[i].end(), numSlots, zContainers);
}
void SpatialGrid::AddSpaceNegZ(uint numSlots) {
   Cell cellContainer = Cell();
   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         cells[i][j].insert(cells[i][j].begin(), numSlots, cellContainer);
}
void SpatialGrid::AddSpacePosZ(uint numSlots) {
   Cell cellContainer = Cell();
   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         cells[i][j].insert(cells[i][j].end(), numSlots, cellContainer);
}

void SpatialGrid::RemoveFromNegX(uint numSlots) {
   int sizeMinDiff = xSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   cells.erase(cells.begin(), cells.begin() + depth);
}
void SpatialGrid::RemoveFromPosX(uint numSlots) {
   int sizeMinDiff = xSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   cells.erase(cells.end() - depth, cells.end());
}
void SpatialGrid::RemoveFromNegY(uint numSlots) {
   int sizeMinDiff = ySize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      cells[i].erase(cells[i].begin(), cells[i].begin() + depth);
}
void SpatialGrid::RemoveFromPosY(uint numSlots) {
   int sizeMinDiff = ySize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      cells[i].erase(cells[i].end() - depth, cells[i].end());
}
void SpatialGrid::RemoveFromNegZ(uint numSlots) {
   int sizeMinDiff = zSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         cells[i][j].erase(cells[i][j].begin(), cells[i][j].begin() + depth);
}
void SpatialGrid::RemoveFromPosZ(uint numSlots) {
   int sizeMinDiff = zSize() - MIN_ACROSS;
   int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

   for (int i = 0; i < xSize(); i++)
      for (int j = 0; j < ySize(); j++)
         cells[i][j].erase(cells[i][j].end() - depth, cells[i][j].end());
}
