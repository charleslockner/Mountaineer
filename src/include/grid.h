// #ifndef __GRID_H__
// #define __GRID_H__

// #include <vector>
// #include <deque>

// #include "model.h"
// #include "matrix_math.h"
// #include "geometry.h"

// typedef unsigned int uint;

// typedef struct {
//    Geom::Positionalf * pnt;
//    float distSq;
// } PointDist;

// class SpatialGrid {
// public:
//    SpatialGrid(int maxAcross, float cellWidth);
//    ~SpatialGrid();

//    uint xSize();
//    uint ySize();
//    uint zSize();
//    int minXIndex();
//    int maxXIndex();
//    int minYIndex();
//    int maxYIndex();
//    int minZIndex();
//    int maxZIndex();
//    uint numElements();

//    void Add(Geom::Positionalf * pnt);
//    void Remove(Geom::Positionalf * pnt);
//    PointDist FindClosestToPoint(Eigen::Vector3f target, float maxDist);
//    PointDist FindClosestToLine(Geom::Rayf line, float maxDist);

// private:
//    class Cell {
//    public:
//       Cell();
//       ~Cell();

//       void Add(Geom::Positionalf * pnt);
//       PointDist FindClosestToPoint(Eigen::Vector3f target);
//       PointDist FindClosestToLine(Geom::Rayf line);

//    private:
//       std::vector<Geom::Positionalf *> _points;
//    };

//    uint _pointCount;
//    uint _maxAcross;
//    float _cellWidth;
//    int _xIndexOffset, _yIndexOffset, _zIndexOffset;
//    std::deque<std::deque<std::deque<Cell> > > _cells;

//    PointDist FindClosestToPointOutward(Eigen::Vector3f target, int cellsOut);
//    PointDist FindNearestToPointInCells(std::vector<Cell *>& checkCells, Eigen::Vector3f point);
//    PointDist FindNearestToLineInCells(std::vector<Cell *>& checkCells, Geom::Rayf line);

//    std::vector<Cell *> findRelevantNeighborCells(Eigen::Vector3f target, float foundDistSq);
//    std::vector<Cell *> findCellsOnLine(Geom::Rayf line, float maxDist);
//    std::vector<Cell *> findCellsOutwardOf(Eigen::Vector3i centerIndexV, int cellsOut);

//    // Helper functions
//    Eigen::Vector3i RealToWorldIndex(Eigen::Vector3i indexV);
//    Eigen::Vector3i WorldToRealIndex(Eigen::Vector3i indexV);
//    // Returns the indices even if they are beyond the bounds of the grid
//    Eigen::Vector3i PointToRealIndex(Eigen::Vector3f pnt);
//    Eigen::Vector3i PointToWorldIndex(Eigen::Vector3f pnt);
//    Eigen::Vector3f worldIndexToNegBounds(Eigen::Vector3i wIndexV);

//    // Returns the cell at the REAL index, or NULL if there is no cell there
//    Cell * GetCellAt(Eigen::Vector3i indexV);
//    // If REAL indexV is beyond the range of the grid, generate new cells
//    // Returns the new REAL indexV, since indices may have shifted
//    Eigen::Vector3i expandIfNeeded(Eigen::Vector3i indexV);

//    // Add a block of cells to 1 of the 6 sides
//    void AddSpaceNegX(uint numSlots);
//    void AddSpacePosX(uint numSlots);
//    void AddSpaceNegY(uint numSlots);
//    void AddSpacePosY(uint numSlots);
//    void AddSpaceNegZ(uint numSlots);
//    void AddSpacePosZ(uint numSlots);

//    // Remove a block of cells from 1 of the 6 sides
//    void RemoveFromNegX(uint numSlots);
//    void RemoveFromPosX(uint numSlots);
//    void RemoveFromNegY(uint numSlots);
//    void RemoveFromPosY(uint numSlots);
//    void RemoveFromNegZ(uint numSlots);
//    void RemoveFromPosZ(uint numSlots);
// };

// #endif // __GRID_H__