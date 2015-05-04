#ifndef __CULLING_H__
#define __CULLING_H__

#include <vector>
#include <array>

#include "matrix_math.h"

class FPlane {
private:
   Eigen::Vector3f normal;
   float D;

public:
   FPlane();
   FPlane(Eigen::Vector3f p1, Eigen::Vector3f p2, Eigen::Vector3f p3);
   float DistanceToPoint(Eigen::Vector3f pt);
};

class ViewFrustrum {
private:
   enum { TOP_P, BOT_P, LEFT_P, RIGHT_P, NEAR_P, FAR_P };
   std::array<FPlane, FAR_P+1> planes;

public:
   enum { NOVIS, VISIBLE };

   void Build(float angle, float ratio, float nearDist, float farDist,
      Eigen::Vector3f camPos, Eigen::Vector3f lookAt, Eigen::Vector3f upVec);
   int IsInside(Eigen::Vector3f center, Eigen::Vector3f scale);
};


#endif // __CULLING_H__