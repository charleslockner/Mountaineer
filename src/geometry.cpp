
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

   Planef::Planef() {}

   Planef::Planef(Eigen::Vector3f pnt, Eigen::Vector3f norm)
   : point(pnt), normal(norm) {}

   Planef::Planef(Eigen::Vector3f a, Eigen::Vector3f b, Eigen::Vector3f c) {
      point = b;
      normal = (c - b).cross(a - b).normalized();
   }

   float Planef::distToPoint(Eigen::Vector3f pnt) {
      return normal.dot(pnt - point);
   }

   Planef operator *(Eigen::Matrix4f transformM, Planef planeOp) {
      return Planef((transformM * Mmath::vec3To4(planeOp.point, 1.0f)).head<3>(),
                    (transformM * Mmath::vec3To4(planeOp.normal, 0.0f)).head<3>());
   }

   Frustumf::Frustumf() {};

   Frustumf::Frustumf(Planef left, Planef right, Planef bottom, Planef top, Planef near, Planef far)
   : left(left), right(right), bottom(bottom), top(top), near(near), far(far) {}

   Frustumf::Frustumf(Eigen::Vector3f nbl, Eigen::Vector3f nbr, Eigen::Vector3f ntl, Eigen::Vector3f ntr,
                      Eigen::Vector3f fbl, Eigen::Vector3f fbr, Eigen::Vector3f ftl, Eigen::Vector3f ftr) {
      left     = Planef(ftl, ntl, nbl);
      right    = Planef(ntr, ftr, fbr);
      bottom   = Planef(fbr, fbl, nbl);
      top      = Planef(ntr, ntl, ftl);
      near     = Planef(ntl, ntr, nbr);
      far      = Planef(ftr, ftl, fbl);
   }

   Frustumf operator *(Eigen::Matrix4f transformM, Frustumf frustOp) {
      return Frustumf(transformM * frustOp.left, transformM * frustOp.right,
                      transformM * frustOp.bottom, transformM * frustOp.top,
                      transformM * frustOp.near, transformM * frustOp.far);
   }

   bool Frustumf::Contains(Eigen::Vector3f pnt) {
      return left.distToPoint(pnt) > 0 && right.distToPoint(pnt) > 0 &&
             bottom.distToPoint(pnt) > 0 && top.distToPoint(pnt) > 0 &&
             near.distToPoint(pnt) > 0 && far.distToPoint(pnt) > 0;
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