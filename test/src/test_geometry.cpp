#include "test.h"
#include "geometry.h"

using namespace Eigen;
using namespace Geom;

void testGeometry() {

   // test Rayf's getPointByDist method
   {
      Rayf ray(Vector3f(0,0,0), Vector3f(1,1,0).normalized());
      Vector3f pnt = ray.getPointByDist(sqrt(2));
      equalityFloatCheck(pnt(0), 1, 1e-5);
      equalityFloatCheck(pnt(1), 1, 1e-5);
      equalityFloatCheck(pnt(2), 0, 1e-5);
   }

   // Test Rayf's squaredDistToPoint method
   {
      Vector3f pnt = Vector3f(1,1,1);
      Rayf ray(Vector3f(0,0,0), Vector3f(0,1,0).normalized());
      float res = ray.squaredDistToPoint(pnt);
      equalityFloatCheck(res, 2, 1e-5);
   }

   // Test Rayf's distToPoint method
   {
      Vector3f pnt = Vector3f(0,0,0);
      Rayf ray(Vector3f(0,-1,0), Vector3f(0,1,0).normalized());
      float res = ray.distToPoint(pnt);
      equalityFloatCheck(res, 0, 1e-5);
   }

   {
      Vector3f pnt = Vector3f(1,1,1);
      Rayf ray(Vector3f(0,0,0), Vector3f(0,1,0).normalized());
      float res = ray.distToPoint(pnt);
      equalityFloatCheck(res, sqrt(2), 1e-5);
   }

   // Test Planef's distToPoint method
   {
      Vector3f pnt = Vector3f(0,0,0);
      Planef plane(Vector3f(0,-1,0), Vector3f(0,1,0).normalized());
      float res = plane.distToPoint(pnt);
      equalityFloatCheck(res, 1, 1e-5);
   }

   {
      Vector3f pnt = Vector3f(0,0,0);
      Planef plane(Vector3f(0,-1,0), Vector3f(0,-1,0).normalized());
      float res = plane.distToPoint(pnt);
      equalityFloatCheck(res, -1, 1e-5);
   }

   {
      Vector3f pnt = Vector3f(0,0,0);
      Planef plane(Vector3f(-2,-2,0), Vector3f(-1,-1,0).normalized());
      float res = plane.distToPoint(pnt);
      equalityFloatCheck(res, -sqrt(8), 1e-5);
   }

   // Test Intersectf (ray and plane)
   {
      Rayf ray(Vector3f(2,2,0), Vector3f(0,-1,0));
      Planef plane(Vector3f(-1,-1,-1), Vector3f(0,1,0));
      Vector3f res = Intersectf(ray, plane);
      equalityFloatCheck(res(0), 2, 1e-5);
      equalityFloatCheck(res(1), -1, 1e-5);
      equalityFloatCheck(res(2), 0, 1e-5);
   }

   {
      Rayf ray(Vector3f(2,2,2), Vector3f(1,0,0));
      Planef plane(Vector3f(-1,-1,-2), Vector3f(0,-1,0));
      Vector3f res = Intersectf(ray, plane);
      nanCheck(res(0));
      nanCheck(res(1));
      nanCheck(res(2));
   }

   {
      Rayf ray(Vector3f(1,0,0), Vector3f(1,1,0));
      Planef plane(Vector3f(0,-1,0), Vector3f(-1,-1,0).normalized());
      Vector3f res = Intersectf(ray, plane);
      equalityFloatCheck(res(0), 0, 1e-5);
      equalityFloatCheck(res(1), -1, 1e-5);
      equalityFloatCheck(res(2), 0, 1e-5);
   }

   {
      Rayf ray(Vector3f(1,0,0), Vector3f(1,1,0));
      Planef plane(Vector3f(0,-1,0), Vector3f(1,1,0).normalized());
      Vector3f res = Intersectf(ray, plane);
      equalityFloatCheck(res(0), 0, 1e-5);
      equalityFloatCheck(res(1), -1, 1e-5);
      equalityFloatCheck(res(2), 0, 1e-5);
   }

   // Test Intersectf (ray and sphere)
   {
      Rayf ray(Vector3f(1,1,0), Vector3f(1,0,0));
      Spheref sphere(Vector3f(0,0,0), 0.5);
      Vector3f res = Intersectf(ray, sphere);
      equalityFloatCheck(res(0), 0, 1e-5);
      equalityFloatCheck(res(1), 0.5, 1e-5);
      equalityFloatCheck(res(2), 0, 1e-5);
   }

   {
      Rayf ray(Vector3f(1,0,0), Vector3f(-1,0,0));
      Spheref sphere(Vector3f(0,0,0), 0.5);
      Vector3f res = Intersectf(ray, sphere);
      equalityFloatCheck(res(0), 0.5, 1e-5);
      equalityFloatCheck(res(1), 0, 1e-5);
      equalityFloatCheck(res(2), 0, 1e-5);
   }

   {
      Rayf ray(Vector3f(4,5,0), Vector3f(-1,0,0));
      Spheref sphere(Vector3f(0,0,0), 0.5);
      Vector3f res = Intersectf(ray, sphere);
      nanCheck(res(0));
      nanCheck(res(1));
      nanCheck(res(2));
   }
}