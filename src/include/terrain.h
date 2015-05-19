#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "matrix_math.h"
#include "geometry.h"
#include "grid.h"

#include <vector>

class Model;
struct Vertex;

class TerrainGenerator {
public:
   TerrainGenerator();
   // Creates and returns the initial model
   Model * GenerateModel();
   // Extends the paths that are within the sphere,
   // Removes the paths that are outside of it
   void UpdateMesh(Eigen::Vector3f center, float radius);
   // Returns the closest point on the mesh to target point
   PointDist FindClosestToPoint(Eigen::Vector3f target);
   // Returns the closest point on the mesh to the line
   PointDist FindClosestToLine(Geom::Linef line);

private:
   class Path: public Geom::Positionalf {
   public:
      enum BuildAction {
         ADVANCE, // path is about to create some geometry
         RETREAT, // path is about to remove some geometry
         STATION  // path is neither creating nor removing geometry
      };

      Vertex *headV, *tailV;
      Path *leftP, *rightP;
      Eigen::Vector3f heading;
      BuildAction buildAction;

      inline void calculateHeading() {
         heading = (headV->position - tailV->position).normalized();
      }
      inline Eigen::Vector3f getPosition() {
         return headV->getPosition();
      }
      inline void setPosition(Eigen::Vector3f pos) {
         headV->setPosition(pos);
      }
   };

   Model * model;
   SpatialGrid * grid;

   std::vector<Path *> paths;
   float edgeLength;

   void BuildStep();
   void PickPathsToExtend(Eigen::Vector3f center, float radius);
   void ExtendPaths();
   void SmoothPathPositions();
   void MergePaths();
   void CreateNeededPaths();
   void AddVerticesAndFaces();
   void RemoveConvergingPaths();

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};

#endif // __TERRAIN_H__