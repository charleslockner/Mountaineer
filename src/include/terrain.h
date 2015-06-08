#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "matrix_math.h"
#include "geometry.h"
#include "grid.h"

#include <vector>

class Model;
struct Vertex;

#define UV_STEP_SIZE .025

class TerrainGenerator {
public:
   TerrainGenerator();
   // Creates and returns the initial model
   Model * GenerateModel();
   // Extends the paths that are within the sphere, and removes the paths that are outside of it
   void UpdateMesh(Eigen::Vector3f center, float radius);

   // // Returns the closest point on the mesh to target point
   // PointDist FindClosestToPoint(Eigen::Vector3f target);
   // // Returns the closest point on the mesh to the line
   // PointDist FindClosestToLine(Geom::Rayf line);

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
      inline void calculateUV() {
         Eigen::Matrix3f iTBN = Mmath::InverseTBN(headV->tangent, headV->bitangent, headV->normal);
         headV->uv = tailV->uv + UV_STEP_SIZE * (iTBN * (headV->position - tailV->position)).head<2>();
      }

      inline Eigen::Vector3f getPosition() {
         return headV->getPosition();
      }
      inline void setPosition(Eigen::Vector3f pos) {
         headV->setPosition(pos);
      }
   };

   std::vector<Path *> paths;
   Model * model;

private:

   SpatialGrid * grid;

   float edgeLength;

   void BuildStep();
   void PickPathsToExtend(Eigen::Vector3f center, float radius);

   void ExtendPaths();
   void MergePaths();
   void CreateNeededPaths();
   void AddVerticesAndFaces();
   void RemoveRetreatingGeometry();
   void RemoveConvergingPaths();
   void CalculateVertexNormals();

   void HandleSameHead(Path * leftP, Path * rightP);
   void HandleSameTail(Path * leftP, Path * rightP);
   void HandleBothDiff(Path * leftP, Path * rightP);
};

#endif // __TERRAIN_H__