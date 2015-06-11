// #include "grid.h"
// #include "assert.h"
// #include "stdio.h"

// #define MIN_ACROSS 1

// // ================================================================== //
// // ======================== STATIC FUNCTIONS ======================== //
// // ================================================================== //

// static inline float square(float n) {
//    return n*n;
// }

// static inline PointDist closerOfPair(PointDist& a, PointDist& b) {
//    if (a.pnt && b.pnt)
//       return (a.distSq < b.distSq) ? a : b;
//    else if (a.pnt && !(b.pnt))
//       return a;
//    else
//       return b;
// }

// // ================================================================== //
// // ======================== PUBLIC FUNCTIONS ======================== //
// // ================================================================== //

// SpatialGrid::SpatialGrid(int maxAcross, float cellWidth)
// : _maxAcross(maxAcross), _cellWidth(cellWidth) {

//    _cells = std::deque<std::deque<std::deque<Cell> > >(MIN_ACROSS);
//    for (int i = 0; i < MIN_ACROSS; i++) {
//       _cells[i] = std::deque<std::deque<Cell> >(MIN_ACROSS);
//       for (int j = 0; j < MIN_ACROSS; j++) {
//          _cells[i][j] = std::deque<Cell>(MIN_ACROSS);
//          for (int k = 0; k < MIN_ACROSS; k++)
//             _cells[i][j][k] = Cell();
//       }
//    }

//    _xIndexOffset = _yIndexOffset = _zIndexOffset = 0;
//    _pointCount = 0;
// }

// SpatialGrid::~SpatialGrid() {}

// uint SpatialGrid::xSize() {
//    return _cells.size();
// }
// uint SpatialGrid::ySize() {
//    return _cells[0].size();
// }
// uint SpatialGrid::zSize() {
//    return _cells[0][0].size();
// }

// // In world indices
// int SpatialGrid::minXIndex() {
//    return 0 - _xIndexOffset;
// }
// int SpatialGrid::maxXIndex() {
//    return xSize() - 1 - _xIndexOffset;
// }
// int SpatialGrid::minYIndex() {
//    return 0 - _yIndexOffset;
// }
// int SpatialGrid::maxYIndex() {
//    return ySize() - 1 - _yIndexOffset;
// }
// int SpatialGrid::minZIndex() {
//    return 0 - _zIndexOffset;
// }
// int SpatialGrid::maxZIndex() {
//    return zSize() - 1 - _zIndexOffset;
// }

// uint SpatialGrid::numElements() {
//    return _pointCount;
// }

// void SpatialGrid::Add(Geom::Positionalf * pnt) {
//    Eigen::Vector3i indexV = PointToRealIndex(pnt->getPosition());
//    Cell * cell = GetCellAt(expandIfNeeded(indexV));
//    assert(cell); // Since we resized to fit the cell, it shouldn't be null;

//    cell->Add(pnt);
//    _pointCount++;
// }

// void SpatialGrid::Remove(Geom::Positionalf * pnt) {

// }

// PointDist SpatialGrid::FindClosestToPoint(Eigen::Vector3f target, float maxDist) {
//    Eigen::Vector3i indexV = PointToRealIndex(target);
//    Cell * cell = GetCellAt(expandIfNeeded(indexV));
//    assert(cell); // Since we resized to fit the cell, it shouldn't be null;

//    PointDist closestPD = cell->FindClosestToPoint(target);

//    // If there is a vertex within the same cell as the target
//    if (closestPD.pnt) {
//       // Find all neighboring cells that need to be checked against the found point
//       std::vector<Cell *> neighCells = findRelevantNeighborCells(target, closestPD.distSq);
//       PointDist neighPD = FindNearestToPointInCells(neighCells, target);
//       return closerOfPair(closestPD, neighPD);
//    }
//    else { // If there are no points within the same cell as the target
//       // return closestPD;
//       return FindClosestToPointOutward(target, 1);
//    }
// }

// PointDist SpatialGrid::FindClosestToLine(Geom::Rayf line, float maxDist) {
//    std::vector<SpatialGrid::Cell *> cells = findCellsOnLine(line, maxDist);
//    return FindNearestToLineInCells(cells, line);
// }

// // ================================================================== //
// // ========================= CELL FUNCTIONS ========================= //
// // ================================================================== //

// SpatialGrid::Cell::Cell() {
//    _points = std::vector<Geom::Positionalf *>(0);
// }
// SpatialGrid::Cell::~Cell() {}

// void SpatialGrid::Cell::Add(Geom::Positionalf * pnt) {
//    _points.push_back(pnt);
// }

// PointDist SpatialGrid::Cell::FindClosestToPoint(Eigen::Vector3f targetPnt) {
//    PointDist shortest;
//    shortest.distSq = -1;
//    shortest.pnt = NULL;

//    if (_points.size() == 0)
//       return shortest;
//    else {
//       shortest.distSq = (targetPnt - _points[0]->getPosition()).squaredNorm();
//       shortest.pnt = _points[0];

//       for (int i = 1; i < _points.size(); i++) {
//          float distSq = (targetPnt - _points[i]->getPosition()).squaredNorm();
//          if (distSq < shortest.distSq) {
//             shortest.distSq = distSq;
//             shortest.pnt = _points[i];
//          }
//       }

//       return shortest;
//    }
// }

// PointDist SpatialGrid::Cell::FindClosestToLine(Geom::Rayf line) {
//    PointDist shortest;
//    shortest.distSq = -1;
//    shortest.pnt = NULL;

//    if (_points.size() == 0)
//       return shortest;
//    else {
//       shortest.distSq = line.squaredDistToPoint(_points[0]->getPosition());
//       shortest.pnt = _points[0];

//       for (int i = 1; i < _points.size(); i++) {
//          float distSq = line.squaredDistToPoint(_points[i]->getPosition());
//          if (distSq < shortest.distSq) {
//             shortest.distSq = distSq;
//             shortest.pnt = _points[i];
//          }
//       }

//       return shortest;
//    }
// }

// // ================================================================== //
// // ======================== PRIVATE FUNCTIONS ======================= //
// // ================================================================== //

// // Recursively searches surrounding cells for the closest point
// PointDist SpatialGrid::FindClosestToPointOutward(Eigen::Vector3f target, int cellsOut) {
//    PointDist closestPD;
//    closestPD.pnt = NULL;
//    closestPD.distSq = -1;

//    if (_pointCount <= 0 || cellsOut >= std::max(std::max(xSize(), ySize()), zSize()))
//       return closestPD;
//    else {
//       Eigen::Vector3i centerIndexV = PointToRealIndex(target);
//       std::vector<Cell *> checkCells = findCellsOutwardOf(centerIndexV, cellsOut);
//       PointDist outPD = FindNearestToPointInCells(checkCells, target);
//       closestPD = closerOfPair(closestPD, outPD);

//       if (closestPD.pnt)
//          return closestPD;
//       else
//          return FindClosestToPointOutward(target, cellsOut + 1);
//    }
// }

// PointDist SpatialGrid::FindNearestToPointInCells(std::vector<Cell *>& checkCells, Eigen::Vector3f point) {
//    PointDist closestPD;
//    closestPD.pnt = NULL;
//    closestPD.distSq = -1;

//    // Compare distances from each cell to find the shortest to the point
//    for (int i = 0; i < checkCells.size(); i++) {
//       PointDist iPD = checkCells[i]->FindClosestToPoint(point);

//       if (closestPD.pnt) {
//          if (iPD.pnt && iPD.distSq < closestPD.distSq)
//             closestPD = iPD;
//       }
//       else if (iPD.pnt) {
//          closestPD = iPD;
//       }
//    }

//    return closestPD;
// }


// PointDist SpatialGrid::FindNearestToLineInCells(std::vector<Cell *>& checkCells, Geom::Rayf line) {
//    PointDist closestPD;
//    closestPD.pnt = NULL;
//    closestPD.distSq = -1;

//    // Compare distances from each cell to find the shortest to the line
//    for (int i = 0; i < checkCells.size(); i++) {
//       PointDist iPD = checkCells[i]->FindClosestToLine(line);

//       if (closestPD.pnt) {
//          if (iPD.pnt && iPD.distSq < closestPD.distSq)
//             closestPD = iPD;
//       }
//       else if (iPD.pnt) {
//          closestPD = iPD;
//       }
//    }

//    return closestPD;
// }

// // Find all neighboring cells that need to be checked against the found point
// std::vector<SpatialGrid::Cell *> SpatialGrid::findRelevantNeighborCells(Eigen::Vector3f target, float foundDistSq) {
//    Eigen::Vector3i indexV = PointToRealIndex(target);
//    Eigen::Vector3i worldIndexV = RealToWorldIndex(indexV);

//    // Calculate the grid boundaries
//    Eigen::Vector3f gridPositionNegV = _cellWidth * worldIndexV.cast<float>();
//    Eigen::Vector3f gridPositionPosV = _cellWidth * (worldIndexV + Eigen::Vector3i(1,1,1)).cast<float>();
//    Eigen::Vector3f distToGridNegV = target - gridPositionNegV;
//    Eigen::Vector3f distToGridPosV = gridPositionPosV - target;

//    // Check to see if any other neighboring cells might contain a closer point
//    // We only have to consider cells who's boundaries are closer than the distance
//    // to the point within the current cell
//    bool checkGridNegX = square(distToGridNegV(0)) < foundDistSq;
//    bool checkGridPosX = square(distToGridPosV(0)) < foundDistSq;
//    bool checkGridNegY = square(distToGridNegV(1)) < foundDistSq;
//    bool checkGridPosY = square(distToGridPosV(1)) < foundDistSq;
//    bool checkGridNegZ = square(distToGridNegV(2)) < foundDistSq;
//    bool checkGridPosZ = square(distToGridPosV(2)) < foundDistSq;

//    // Start off needing to check all surrounding cells
//    int needToCheck[3][3][3];
//    for (int i = 0; i < 3; i++)
//       for (int j = 0; j < 3; j++)
//          for (int k = 0; k < 3; k++)
//             needToCheck[i][j][k] = true;
//    // Cross off sections based on checkGrid booleans
//    for (int j = 0; j < 3; j++)
//       for (int k = 0; k < 3; k++)
//          if (!checkGridNegX)
//             needToCheck[0][j][k] = false;
//    for (int j = 0; j < 3; j++)
//       for (int k = 0; k < 3; k++)
//          if (!checkGridPosX)
//             needToCheck[2][j][k] = false;
//    for (int i = 0; i < 3; i++)
//       for (int k = 0; k < 3; k++)
//          if (!checkGridNegY)
//             needToCheck[i][0][k] = false;
//    for (int i = 0; i < 3; i++)
//       for (int k = 0; k < 3; k++)
//          if (!checkGridPosY)
//             needToCheck[i][2][k] = false;
//    for (int i = 0; i < 3; i++)
//       for (int j = 0; j < 3; j++)
//          if (!checkGridNegZ)
//             needToCheck[i][j][0] = false;
//    for (int i = 0; i < 3; i++)
//       for (int j = 0; j < 3; j++)
//          if (!checkGridPosZ)
//             needToCheck[i][j][2] = false;
//    // We already have a value for the center, so we don't need to check this
//    needToCheck[1][1][1] = false;

//    std::vector<Cell *> checkCells = std::vector<Cell *>(0);
//    checkCells.reserve(26);

//    // Add all the cells we need to check to a vector
//    for (int i = 0; i < 3; i++) {
//       for (int j = 0; j < 3; j++) {
//          for (int k = 0; k < 3; k++) {
//             if (needToCheck[i][j][k]) {
//                Cell * cell = GetCellAt(indexV + Eigen::Vector3i(i-1,j-1,k-1));
//                if (cell)
//                   checkCells.push_back(cell);
//             }
//          }
//       }
//    }

//    return checkCells;
// }

// std::vector<SpatialGrid::Cell *> SpatialGrid::findCellsOutwardOf(Eigen::Vector3i centerIndexV, int cellsOut) {
//    std::vector<Cell *> checkCells = std::vector<Cell *>(0);
//    int sideLen = 2 * cellsOut + 1;
//    Cell * cell;

//    // printf("findCellsOutwardOf center %d %d %d\n", centerIndexV(0), centerIndexV(1), centerIndexV(2));

//    // Add the left and right sides
//    // printf("sizes %d %d %d\n", xSize(), ySize(), zSize());
//    // printf("-sidelen/2 = %d\n", -sideLen/2);
//    // printf("sidelen/2 = %d\n", sideLen/2);
//    for (int j = -sideLen/2; j <= sideLen/2; j++) {
//       // printf("j = %d\n", j);
//       for (int k = -sideLen/2; k <= sideLen/2; k++) {
//          // printf("k = %d\n", k);
//          // printf("index %d %d %d\n", (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(0), (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(1), (centerIndexV + Eigen::Vector3i(-cellsOut, j, k))(2));
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(-cellsOut, j, k));
//          if (cell)
//             checkCells.push_back(cell);
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(cellsOut, j, k));
//          if (cell)
//             checkCells.push_back(cell);
//          // if (cell)
//          //    printf("there's a cell here! %d %d\n", j, k);
//       }
//    }
//    // Add the top and bottom sides
//    for (int i = -sideLen/2+1; i <= sideLen/2-1; i++) {
//       for (int k = -sideLen/2; k < sideLen/2; k++) {
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, -cellsOut, k));
//          if (cell)
//             checkCells.push_back(cell);
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, cellsOut, k));
//          if (cell)
//             checkCells.push_back(cell);
//       }
//    }
//    // Add the front and back sides
//    for (int i = -sideLen/2+1; i <= sideLen/2-1; i++) {
//       for (int j = -sideLen/2+1; j < sideLen/2-1; j++) {
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, j, -cellsOut));
//          if (cell)
//             checkCells.push_back(cell);
//          cell = GetCellAt(centerIndexV + Eigen::Vector3i(i, j, cellsOut));
//          if (cell)
//             checkCells.push_back(cell);
//       }
//    }

//    return checkCells;
// }

// static bool cellsLeft(Eigen::Vector3i boundOffV, Eigen::Vector3i indexV, Eigen::Vector3i endIndexV) {
//    return ((!boundOffV(0) && indexV(0) >= endIndexV(0)) ||
//            ( boundOffV(0) && indexV(0) <= endIndexV(0)) ) &&
//           ((!boundOffV(1) && indexV(1) >= endIndexV(1)) ||
//            ( boundOffV(1) && indexV(1) <= endIndexV(1)) ) &&
//           ((!boundOffV(2) && indexV(2) >= endIndexV(2)) ||
//            ( boundOffV(2) && indexV(2) <= endIndexV(2)) );
// }

// std::vector<SpatialGrid::Cell *> SpatialGrid::findCellsOnLine(Geom::Rayf line, float maxDist) {
//    Eigen::Vector3i boundOffV = Eigen::Vector3i( (line.direction(0) >= 0 ? 1 : 0),
//                                             (line.direction(1) >= 0 ? 1 : 0),
//                                             (line.direction(2) >= 0 ? 1 : 0) );
//    Eigen::Vector3i incrV = Eigen::Vector3i( (boundOffV(0) ? 1 : -1),
//                                             (boundOffV(1) ? 1 : -1),
//                                             (boundOffV(2) ? 1 : -1) );

//    std::vector<Cell *> checkCells = std::vector<Cell *>(0);

//    Eigen::Vector3i endIndexV = PointToWorldIndex(line.getPointByDist(maxDist));
//    Eigen::Vector3i indexV = PointToWorldIndex(line.start);
//    Eigen::Vector3f boundsV = worldIndexToNegBounds(indexV + boundOffV);

//    Geom::Planef nextPlaneX = Geom::Planef(Eigen::Vector3f(boundsV(0), 0, 0), Eigen::Vector3f(1,0,0));
//    Geom::Planef nextPlaneY = Geom::Planef(Eigen::Vector3f(0, boundsV(1), 0), Eigen::Vector3f(0,1,0));
//    Geom::Planef nextPlaneZ = Geom::Planef(Eigen::Vector3f(0, 0, boundsV(2)), Eigen::Vector3f(0,0,1));

//    // Move 1 cell forward each iteration and add it to the list
//    while (cellsLeft(boundOffV, indexV, endIndexV)) {
//       Cell * cell = GetCellAt(expandIfNeeded(WorldToRealIndex(indexV)));
//       assert(cell);
//       checkCells.push_back(cell);

//       float xNextDist = line.squaredDistToPoint(Geom::Intersectf(line, nextPlaneX));
//       float yNextDist = line.squaredDistToPoint(Geom::Intersectf(line, nextPlaneY));
//       float zNextDist = line.squaredDistToPoint(Geom::Intersectf(line, nextPlaneZ));

//       if (xNextDist < yNextDist && xNextDist < zNextDist)
//          indexV(0) += incrV(0);
//       else if (yNextDist < xNextDist && yNextDist < zNextDist)
//          indexV(1) += incrV(1);
//       else
//          indexV(2) += incrV(2);

//       boundsV = worldIndexToNegBounds(indexV + boundOffV);
//       nextPlaneX = Geom::Planef(Eigen::Vector3f(boundsV(0), 0, 0), Eigen::Vector3f(1,0,0));
//       nextPlaneY = Geom::Planef(Eigen::Vector3f(0, boundsV(1), 0), Eigen::Vector3f(0,1,0));
//       nextPlaneZ = Geom::Planef(Eigen::Vector3f(0, 0, boundsV(2)), Eigen::Vector3f(0,0,1));
//    }

//    return checkCells;
// }

// // ================================================================== //
// // ===================== SHARED HELPER FUNCTIONS ==================== //
// // ================================================================== //

// Eigen::Vector3i SpatialGrid::RealToWorldIndex(Eigen::Vector3i indexV) {
//    return indexV - Eigen::Vector3i(_xIndexOffset, _yIndexOffset, _zIndexOffset);
// }

// Eigen::Vector3i SpatialGrid::WorldToRealIndex(Eigen::Vector3i indexV) {
//    return indexV + Eigen::Vector3i(_xIndexOffset, _yIndexOffset, _zIndexOffset);
// }

// Eigen::Vector3i SpatialGrid::PointToRealIndex(Eigen::Vector3f pnt) {
//    int xIndex = int(pnt(0) / _cellWidth) + (pnt(0) < 0 ? -1 : 0) + _xIndexOffset;
//    int yIndex = int(pnt(1) / _cellWidth) + (pnt(1) < 0 ? -1 : 0) + _yIndexOffset;
//    int zIndex = int(pnt(2) / _cellWidth) + (pnt(2) < 0 ? -1 : 0) + _zIndexOffset;
//    return Eigen::Vector3i(xIndex, yIndex, zIndex);
// }

// Eigen::Vector3i SpatialGrid::PointToWorldIndex(Eigen::Vector3f pnt) {
//    int xIndex = int(pnt(0) / _cellWidth) + (pnt(0) < 0 ? -1 : 0);
//    int yIndex = int(pnt(1) / _cellWidth) + (pnt(1) < 0 ? -1 : 0);
//    int zIndex = int(pnt(2) / _cellWidth) + (pnt(2) < 0 ? -1 : 0);
//    return Eigen::Vector3i(xIndex, yIndex, zIndex);
// }

// Eigen::Vector3f SpatialGrid::worldIndexToNegBounds(Eigen::Vector3i wIndexV) {
//    return _cellWidth * wIndexV.cast<float>();
// }


// SpatialGrid::Cell * SpatialGrid::GetCellAt(Eigen::Vector3i indexV) {
//    if ( indexV(0) < 0 || indexV(0) >= xSize() ||
//         indexV(1) < 0 || indexV(1) >= ySize() ||
//         indexV(2) < 0 || indexV(2) >= zSize() )
//       return NULL;
//    else
//       return & _cells[indexV(0)][indexV(1)][indexV(2)];
// }

// Eigen::Vector3i SpatialGrid::expandIfNeeded(Eigen::Vector3i indexV) {
//    int xIndex = indexV(0);
//    int yIndex = indexV(1);
//    int zIndex = indexV(2);

//    if (xIndex < 0) {
//       AddSpaceNegX(-xIndex);
//       _xIndexOffset += -xIndex;
//       xIndex += -xIndex;
//    }
//    else if (xIndex >= xSize()) {
//       AddSpacePosX(xIndex - xSize() + 1);
//    }

//    if (yIndex < 0) {
//       AddSpaceNegY(-yIndex);
//       _yIndexOffset += -yIndex;
//       yIndex += -yIndex;
//    }
//    else if (yIndex >= ySize()) {
//       AddSpacePosY(yIndex - ySize() + 1);
//    }

//    if (zIndex < 0) {
//       AddSpaceNegZ(-zIndex);
//       _zIndexOffset += -zIndex;
//       zIndex += -zIndex;
//    }
//    else if (zIndex >= zSize()) {
//       AddSpacePosZ(zIndex - zSize() + 1);
//    }

//    return Eigen::Vector3i(xIndex, yIndex, zIndex);
// }

// void SpatialGrid::AddSpaceNegX(uint numSlots) {
//    std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
//    for (int j = 0; j < ySize(); j++) {
//       yzContainers[j] = std::deque<Cell>(zSize());
//       for (int k = 0; k < zSize(); k++)
//          yzContainers[j][k] = Cell();
//    }

//    _cells.insert(_cells.begin(), numSlots, yzContainers);
// }
// void SpatialGrid::AddSpacePosX(uint numSlots) {
//    std::deque<std::deque<Cell> > yzContainers = std::deque<std::deque<Cell> >(ySize());
//    for (int j = 0; j < ySize(); j++) {
//       yzContainers[j] = std::deque<Cell>(zSize());
//       for (int k = 0; k < zSize(); k++)
//          yzContainers[j][k] = Cell();
//    }

//    _cells.insert(_cells.end(), numSlots, yzContainers);
// }
// void SpatialGrid::AddSpaceNegY(uint numSlots) {
//    std::deque<Cell> zContainers = std::deque<Cell>(zSize());
//    for (int k = 0; k < zSize(); k++)
//       zContainers[k] = Cell();

//    for (int i = 0; i < xSize(); i++)
//       _cells[i].insert(_cells[i].begin(), numSlots, zContainers);
// }
// void SpatialGrid::AddSpacePosY(uint numSlots) {
//    std::deque<Cell> zContainers = std::deque<Cell>(zSize());
//    for (int k = 0; k < zSize(); k++)
//       zContainers[k] = Cell();

//    for (int i = 0; i < xSize(); i++)
//       _cells[i].insert(_cells[i].end(), numSlots, zContainers);
// }
// void SpatialGrid::AddSpaceNegZ(uint numSlots) {
//    Cell cellContainer = Cell();
//    for (int i = 0; i < xSize(); i++)
//       for (int j = 0; j < ySize(); j++)
//          _cells[i][j].insert(_cells[i][j].begin(), numSlots, cellContainer);
// }
// void SpatialGrid::AddSpacePosZ(uint numSlots) {
//    Cell cellContainer = Cell();
//    for (int i = 0; i < xSize(); i++)
//       for (int j = 0; j < ySize(); j++)
//          _cells[i][j].insert(_cells[i][j].end(), numSlots, cellContainer);
// }

// void SpatialGrid::RemoveFromNegX(uint numSlots) {
//    int sizeMinDiff = xSize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    _cells.erase(_cells.begin(), _cells.begin() + depth);
// }
// void SpatialGrid::RemoveFromPosX(uint numSlots) {
//    int sizeMinDiff = xSize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    _cells.erase(_cells.end() - depth, _cells.end());
// }
// void SpatialGrid::RemoveFromNegY(uint numSlots) {
//    int sizeMinDiff = ySize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    for (int i = 0; i < xSize(); i++)
//       _cells[i].erase(_cells[i].begin(), _cells[i].begin() + depth);
// }
// void SpatialGrid::RemoveFromPosY(uint numSlots) {
//    int sizeMinDiff = ySize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    for (int i = 0; i < xSize(); i++)
//       _cells[i].erase(_cells[i].end() - depth, _cells[i].end());
// }
// void SpatialGrid::RemoveFromNegZ(uint numSlots) {
//    int sizeMinDiff = zSize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    for (int i = 0; i < xSize(); i++)
//       for (int j = 0; j < ySize(); j++)
//          _cells[i][j].erase(_cells[i][j].begin(), _cells[i][j].begin() + depth);
// }
// void SpatialGrid::RemoveFromPosZ(uint numSlots) {
//    int sizeMinDiff = zSize() - MIN_ACROSS;
//    int depth = numSlots < sizeMinDiff ? numSlots : sizeMinDiff;

//    for (int i = 0; i < xSize(); i++)
//       for (int j = 0; j < ySize(); j++)
//          _cells[i][j].erase(_cells[i][j].end() - depth, _cells[i][j].end());
// }
