#ifndef __IK_SOLVER_H__
#define __IK_SOLVER_H__

#include "matrix_math.h"
#include "model.h"
#include "ceres/ceres.h"
#include <vector>

struct Model;
class LimbCostFunctor;

class IKSolver {
public:
   IKSolver(Model * model, std::vector<short> boneIndices);
   ~IKSolver();

   void solveBoneRotations(
      Eigen::Matrix4f baseM,
      Eigen::Vector3f goal,
      std::vector<float *> angles
   );

   std::vector<short> boneIndices;

private:
   Model * model;
};

#endif // __IK_SOLVER_H__
