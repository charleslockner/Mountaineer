#ifndef __IK_SOLVER_H__
#define __IK_SOLVER_H__

#include "matrix_math.h"
#include "model.h"
#include "ceres/ceres.h"
#include <vector>

struct Model;
class LimbCostFunctor;

namespace IK {
   void SolveSegment(
      Model * model,
      Eigen::Matrix4f baseM,
      Eigen::Vector3f goal,
      std::vector<float *> angles,
      std::vector<short> boneIndices
   );

   void SolveThroughRoot(
      Model * model,
      Eigen::Matrix4f baseSclRotM,
      Eigen::Vector3f * basePosition,
      Eigen::Vector3f baseGoal,
      Eigen::Vector3f reachGoal,
      std::vector<float *> baseAngles,
      std::vector<float *> reachAngles,
      std::vector<short> baseBoneIndices,
      std::vector<short> reachBoneIndices
   );
}

#endif // __IK_SOLVER_H__
