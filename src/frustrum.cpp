#include "culling.h"
#include <algorithm>
#include <stdio.h>

// references:
// http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
// http://www.cescg.org/CESCG-2002/DSykoraJJelinek/
// Zoe's handouts
// http://www.lighthouse3d.com/tutorials/view-frustum-culling/

#define WIDTH  0
#define HEIGHT 1

#define NEAR  0
#define FAR   1

#define TOP   0
#define BOT   1

#define LEFT  0
#define RIGHT 1

FPlane::FPlane() {}
FPlane::FPlane(Eigen::Vector3f p1, Eigen::Vector3f p2, Eigen::Vector3f p3) {
   normal = ((p3 - p2).cross(p1 - p2)).normalized();
   D = normal.dot(p2);
}

float FPlane::DistanceToPoint(Eigen::Vector3f pt) {
   return normal.dot(pt) - D;
}

// takes the same args as glm::perspective and glm::lookAt combined (maybe should just take the resulting matrix?)
// resets the camera's viewing frustrum and prepares it for cullings
void ViewFrustrum::Build(float angle, float ratio, float nearDist, float farDist,
         Eigen::Vector3f camPos, Eigen::Vector3f lookAt, Eigen::Vector3f upVec) {

   float tang = tan(angle * M_PI/360.0);

   float near[2]; // width/height
   float far[2];  // width/height

   near[HEIGHT] = nearDist * tang;
   near[WIDTH]  = near[HEIGHT] * ratio;
   far[HEIGHT]  = farDist  * tang;
   far[WIDTH]   = far[HEIGHT] * ratio;

   Eigen::Vector3f nCent, fCent, A, B, C;

   // the amount of offset +/- in Z
   C = (camPos - lookAt).normalized();
   // the amount of offset +/- in Y
   A = (upVec.cross(C)).normalized();
   // the amount of offset +/- in X
   B = C.cross(A);

   // near plane center point
   nCent = camPos - C * nearDist;
   // far  plane center point
   fCent = camPos - C * farDist;

   // this would probably be optimized out anyway with local value propogation
   // but we can try to do it manually in case GCC doesn't do it correctly
   Eigen::Vector3f bNearHeight = B * near[HEIGHT];
   Eigen::Vector3f aNearWidth  = A * near[WIDTH];
   Eigen::Vector3f bFarHeight  = B * far[HEIGHT];
   Eigen::Vector3f aFarWidth   = A * far[WIDTH];

   // build the view frustrum in a contingous array
   // [near/far][top/bottom][left/right]
   std::array<std::array<std::array<Eigen::Vector3f, 2>, 2>, 2> pts = std::array<std::array<std::array<Eigen::Vector3f, 2>, 2>, 2> {{
      {{ {{nCent + bNearHeight - aNearWidth, nCent + bNearHeight + aNearWidth}} ,
         {{nCent - bNearHeight - aNearWidth, nCent - bNearHeight + aNearWidth}} }},
      {{ {{fCent + bFarHeight - aFarWidth,   fCent + bFarHeight + aFarWidth}} ,
         {{fCent - bFarHeight - aFarWidth,   fCent - bFarHeight + aFarWidth}} }}
   }};

   planes = std::array<FPlane, FAR_P+1>({{
      FPlane(pts[NEAR][TOP][RIGHT], pts[NEAR][TOP][LEFT],  pts[FAR][TOP][LEFT]),   // TOP_P
      FPlane(pts[NEAR][BOT][LEFT],  pts[NEAR][BOT][RIGHT], pts[FAR][BOT][RIGHT]),  // BOT_P
      FPlane(pts[NEAR][TOP][LEFT],  pts[NEAR][BOT][LEFT],  pts[FAR][BOT][LEFT] ),  // LEFT_P
      FPlane(pts[NEAR][BOT][RIGHT], pts[NEAR][TOP][RIGHT], pts[FAR][BOT][RIGHT]),  // RIGHT_P
      FPlane(pts[NEAR][TOP][LEFT],  pts[NEAR][TOP][RIGHT], pts[NEAR][BOT][RIGHT]), // NEAR_P
      FPlane(pts[FAR][TOP][RIGHT],  pts[FAR][TOP][LEFT],   pts[FAR][BOT][LEFT])    // FAR_P
   }});
}

// check the entities position and "edible scale" (its active dimensions) to see if
// it is contained in the viewing frustrum
// right now we just return NOVIS/VISIBLE but could return an "INTERSECT" and splice the geometry up
// and try to render a smaller portion of it
int ViewFrustrum::IsInside(Eigen::Vector3f center, Eigen::Vector3f scale) {
   float distance;

   for (auto pl : planes) {
      distance = pl.DistanceToPoint(center);
      // if the distance is -1*radius from ANY plane then we know it is wholly outside the frustrum
      // cull it
      if (distance < -1 * std::max(scale(0), std::max(scale(2), 1.f)))
         return NOVIS;
   }

   return VISIBLE;
}