
#include "geometry.h"

namespace Geom {

   Rayf::Rayf(Eigen::Vector3f pnt, Eigen::Vector3f dir)
   : start(pnt), direction(dir) {}

   Eigen::Vector3f Rayf::getPointByDist(float dist) {
      return start + dist * direction;
   }

   float Rayf::distToPoint(Eigen::Vector3f pnt) {
      return (pnt - start).cross(pnt - start - direction).norm();
   }

   float Rayf::squaredDistToPoint(Eigen::Vector3f pnt) {
      return (pnt - start).cross(pnt - start - direction).squaredNorm();
   }

   Planef::Planef(Eigen::Vector3f pnt, Eigen::Vector3f norm)
   : point(pnt), normal(norm) {}

   float Planef::distToPoint(Eigen::Vector3f pnt) {
      return normal.dot(pnt - point);
   }

   Spheref::Spheref(Eigen::Vector3f cent, float rad)
   : center(cent), radius(rad) {}

   Eigen::Vector3f Intersectf(Rayf ray, Planef plane) {
      float t = - plane.normal.dot(ray.start - plane.point) /
                  plane.normal.dot(ray.direction);
      return ray.getPointByDist(t);
   }

   Eigen::Vector3f Intersectf(Rayf ray, Spheref sphere) {
      float t = ray.direction.dot(sphere.center - ray.start);
      Eigen::Vector3f pClose = ray.getPointByDist(t);

      float radiusSq = sphere.radius * sphere.radius;
      float pDistSq = pClose.squaredNorm();

      if (pDistSq > radiusSq) // no hit
         return Eigen::Vector3f(NAN, NAN, NAN);
      else if (pDistSq == sphere.radius) // hit sphere tangentially
         return pClose;
      else // hit through the sphere
         return ray.getPointByDist(t - sqrt(radiusSq - pDistSq));
   }
}