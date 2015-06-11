#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "matrix_math.h"

namespace Geom {

   class Pointf {
   public:
      Eigen::Vector3f position;

      Pointf(Eigen::Vector3f pos)
      : position(pos) {}
   };

   class Rayf {
   public:
      Eigen::Vector3f start;
      Eigen::Vector3f direction;

      Rayf(Eigen::Vector3f pnt, Eigen::Vector3f dir);
      Eigen::Vector3f getPointByDist(float dist);
      float distToPoint(Eigen::Vector3f pnt);
      float squaredDistToPoint(Eigen::Vector3f pnt);
   };

   class Planef {
   public:
      Eigen::Vector3f point;
      Eigen::Vector3f normal;

      Planef(Eigen::Vector3f pnt, Eigen::Vector3f norm);
      float distToPoint(Eigen::Vector3f pnt);
   };

   class Spheref {
   public:
      Eigen::Vector3f center;
      float radius;

      Spheref(Eigen::Vector3f cent, float rad);
   };

   // Find the point in which the ray intersects the plane
   Eigen::Vector3f Intersectf(Rayf ray, Planef plane);
   Eigen::Vector3f Intersectf(Rayf ray, Spheref sphere);

}

#endif // __GEOMETRY_H__
