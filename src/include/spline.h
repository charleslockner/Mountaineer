#ifndef __SPLINE_H__
#define __SPLINE_H__

#include "matrix_math.h"
#include <vector>

class Spline {
public:
   typedef enum BasisType {
      LINEAR,
      BEZIER,
      CATMULL,
      B
   } BasisType;

   Spline(std::vector<Eigen::Vector3f> cps, BasisType type);
   Spline(BasisType type);

   void addControlPoint(Eigen::Vector3f pnt);
   Eigen::Vector3f getPositionAtDistance(float dist);
   float getTotalDistance();

private:
   Eigen::Matrix4f _basisM;
   BasisType _type;
   float _maxDist;
   std::vector< Eigen::Vector3f > _cps;
   std::vector< std::pair<float, float> > _usTable;

   float distToU(float dist);
   Eigen::Vector3f getPointAtGlobalU(float globU);

   void buildTable();
   float distBetweenU(Eigen::MatrixXf G, float uA, float uB);
   Eigen::MatrixXf createGMatrix(int startI);
};

#endif // __SPLINE_H__