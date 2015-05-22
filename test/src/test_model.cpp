#include "test.h"
#include "model.h"

using namespace Eigen;
using namespace Geom;

void testModel() {
   {
      // Test Face
      Vertex v1;
      v1.position = Vector3f(  0, .5,0);
      Vertex v2;
      v2.position = Vector3f(-.5,-.5,0);
      Vertex v3;
      v3.position = Vector3f( .5,-.5,0);
      Face f;
      f.vertices[0] = & v1;
      f.vertices[1] = & v2;
      f.vertices[2] = & v3;

      // Test calculateNormal
      f.calculateNormal();
      equalityFloatCheck(f.normal(0), 0, 1e-5);
      equalityFloatCheck(f.normal(1), 0, 1e-5);
      equalityFloatCheck(f.normal(2), 1, 1e-5);

      {
         // Test intersectRay
         Rayf ray(Vector3f(0,0,5), Vector3f(0,0,-1));
         Vector3f pnt = f.intersectRay(ray);
         equalityFloatCheck(pnt(0), 0, 1e-5);
         equalityFloatCheck(pnt(1), 0, 1e-5);
         equalityFloatCheck(pnt(2), 0, 1e-5);

         // Test pointCheckInside
         bool isInside = f.pointCheckInside(pnt);
         boolCheck(isInside, true);
      }

      {
         // Test intersectRay
         Rayf ray(Vector3f(3,3,5), Vector3f(0,0,-1));
         Vector3f pnt = f.intersectRay(ray);
         equalityFloatCheck(pnt(0), 3, 1e-5);
         equalityFloatCheck(pnt(1), 3, 1e-5);
         equalityFloatCheck(pnt(2), 0, 1e-5);

         // Test pointCheckInside
         bool isInside = f.pointCheckInside(pnt);
         boolCheck(isInside, false);
      }

      {
         // Test intersectRay
         Rayf ray(Vector3f(-3,3,5), Vector3f(0,0,-1));
         Vector3f pnt = f.intersectRay(ray);
         equalityFloatCheck(pnt(0), -3, 1e-5);
         equalityFloatCheck(pnt(1), 3, 1e-5);
         equalityFloatCheck(pnt(2), 0, 1e-5);

         // Test pointCheckInside
         bool isInside = f.pointCheckInside(pnt);
         boolCheck(isInside, false);
      }

      {
         // Test intersectRay
         Rayf ray(Vector3f(0,-3,5), Vector3f(0,0,-1));
         Vector3f pnt = f.intersectRay(ray);
         equalityFloatCheck(pnt(0), 0, 1e-5);
         equalityFloatCheck(pnt(1), -3, 1e-5);
         equalityFloatCheck(pnt(2), 0, 1e-5);

         // Test pointCheckInside
         bool isInside = f.pointCheckInside(pnt);
         boolCheck(isInside, false);
      }
   }
}