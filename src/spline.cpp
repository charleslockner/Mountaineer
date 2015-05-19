#include "spline.h"
#include <exception>
#include <iostream>
#include <cmath>

// number of segments per control point
#define N_SUBSEGS 5

// ========================================================= //
// ==================== STATIC FUNCTIONS =================== //
// ========================================================= //

static inline Eigen::Matrix4f chooseBasis(Spline::BasisType type) {
   Eigen::Matrix4f B;

   if (type == Spline::BasisType::BEZIER) {
      B <<  1.0f, -3.0f,  3.0f, -1.0f,
            4.0f,  0.0f, -6.0f,  3.0f,
            1.0f,  3.0f,  3.0f, -3.0f,
            0.0f,  0.0f,  0.0f,  1.0f;
   }
   else if (type == Spline::BasisType::CATMULL) {
      B <<  0.0f, -1.0f,  2.0f, -1.0f,
            2.0f,  0.0f, -5.0f,  3.0f,
            0.0f,  1.0f,  4.0f, -3.0f,
            0.0f,  0.0f, -1.0f,  1.0f;
      B = (1.0/2) * B;
   }
   else if (type == Spline::BasisType::B) {
      B <<  1.0f, -3.0f,  3.0f, -1.0f,
            4.0f,  0.0f, -6.0f,  3.0f,
            1.0f,  3.0f,  3.0f, -3.0f,
            0.0f,  0.0f,  0.0f,  1.0f;
      B = (1.0/6) * B;
   }
   else {
      B = Eigen::Matrix4f::Identity();
   }

   return B;
}

// ========================================================= //
// ==================== PUBLIC FUNCTIONS =================== //
// ========================================================= //

Spline::Spline(std::vector<Eigen::Vector3f> cps, BasisType type) {
   _cps = cps;
   _type = type;
   _basisM = chooseBasis(type);
   if (cps.size() > 4)
      buildTable();

   // Print out the table
   for(int i = 0; i < _usTable.size(); i++) {
      std::pair<float,float> row = _usTable[i];
      std::cout << row.first << ", " << row.second << std::endl;
   }
}

Spline::Spline(BasisType type) {
   _cps = std::vector<Eigen::Vector3f>();
   _type = type;
   _basisM = chooseBasis(type);
}

void Spline::addControlPoint(Eigen::Vector3f pnt) {
   _cps.push_back(pnt);
   if (_cps.size() > 4)
      buildTable();
}

Eigen::Vector3f Spline::getPositionAtDistance(float dist) {
   return getPointAtGlobalU(distToU(dist));
}

float Spline::getTotalDistance() {
   return _maxDist;
}

// ========================================================= //
// =================== PRIVATE FUNCTIONS =================== //
// ========================================================= //

float Spline::distToU(float dist) {
   int lateI;
   float lateDist;
   for (lateI = 1; (lateDist = _usTable[lateI].second) < dist; lateI++);
   int earlyI = lateI-1;
   float earlyDist = _usTable[earlyI].second;

   float earlyU = _usTable[earlyI].first;
   float lateU = _usTable[lateI].first;

   float perc = (dist - earlyDist) / (lateDist - earlyDist);
   return (1 - perc) * earlyU + perc * lateU;
}

Eigen::Vector3f Spline::getPointAtGlobalU(float globU) {
   int ndxFirst = (int)globU;
   float u = globU - ndxFirst;

   Eigen::MatrixXf G = createGMatrix(ndxFirst);
   Eigen::Vector4f uVec(1.0f, u, u*u, u*u*u);

   return G * _basisM * uVec;
}

void Spline::buildTable() {
   // Start the table off with 1 pair at (0,0)
   _usTable.clear();
   _maxDist = 0.0f;
   _usTable.push_back(std::make_pair(0.0f, 0.0f));

   // Do the rest of the pairs
   for(int i = 0; i < _cps.size() - 3; i++) {
      Eigen::MatrixXf G = createGMatrix(i);

      for (int l = 1; l <= N_SUBSEGS; l++) {
         float uLocalA = 1.0f * (l-1) / N_SUBSEGS;
         float uLocalB = 1.0f * l / N_SUBSEGS;

         float uTot = i + uLocalB;
         _maxDist += distBetweenU(G, uLocalA, uLocalB);
         _usTable.push_back(std::make_pair(uTot, _maxDist));
      }
   }
}

// Guassian Quadrature Summation for Arc Length Parameterization
// uB must be greater than uA
float Spline::distBetweenU(Eigen::MatrixXf G, float uA, float uB) {
   float uDiffHalf = (uB - uA)/2;
   float uAverage  = (uA + uB)/2;

   float lateU = uDiffHalf * -0.775f + uAverage;
   float u2 = uDiffHalf *  0.000f + uAverage;
   float u3 = uDiffHalf *  0.775f + uAverage;

   Eigen::Vector4f uVec1 = Eigen::Vector4f(0.0f, 1, 2*lateU, 3*lateU*lateU);
   Eigen::Vector4f uVec2 = Eigen::Vector4f(0.0f, 1, 2*u2, 3*u2*u2);
   Eigen::Vector4f uVec3 = Eigen::Vector4f(0.0f, 1, 2*u3, 3*u3*u3);

   Eigen::Vector3f P1 = G * _basisM * uVec1;
   Eigen::Vector3f P2 = G * _basisM * uVec2;
   Eigen::Vector3f P3 = G * _basisM * uVec3;

   float sum1 = (5.0f/9.0f) * P1.norm();
   float sum2 = (8.0f/9.0f) * P2.norm();
   float sum3 = (5.0f/9.0f) * P3.norm();

   return uDiffHalf * (sum1 + sum2 + sum3);
}

Eigen::MatrixXf Spline::createGMatrix(int startI) {
   Eigen::MatrixXf G(3,4);
   for (int i = 0; i < 4; i++)
      G.col(i) = _cps[startI + i];
   return G;
}
