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

      Planef();
      Planef(Eigen::Vector3f pnt, Eigen::Vector3f norm);
      // Constructs a plane who's normal is defined by (c - b)x(a - b)
      // Wind counterclockwise for normal to face inwards or towards you
      Planef(Eigen::Vector3f a, Eigen::Vector3f b, Eigen::Vector3f c);

      // Returns a positive distance if the point is on the same side as the direction of the normal.
      // Returns a negative distance if the point is on the other side of the plane
      float distToPoint(Eigen::Vector3f pnt);
   };

   class Frustumf {
   public:
      Planef left, right;
      Planef bottom, top;
      Planef near, far;

      Frustumf();
      Frustumf(Planef left, Planef right, Planef bottom, Planef top, Planef near, Planef far);
      Frustumf(Eigen::Vector3f nbl, Eigen::Vector3f nbr, Eigen::Vector3f ntl, Eigen::Vector3f ntr,
               Eigen::Vector3f fbl, Eigen::Vector3f fbr, Eigen::Vector3f ftl, Eigen::Vector3f ftr);
      bool Contains(Eigen::Vector3f pnt);
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
