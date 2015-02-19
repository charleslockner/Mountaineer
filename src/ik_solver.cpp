
#include <iostream>

#include "ik_solver.h"
#include "ceres/ceres.h"

struct CostFunctor {
   template <typename T> bool operator()(const T* const x, T* residual) const {

      residual[0] = (T(0.2))*(x[0]*x[0] + x[1]*x[1]) + (sin((T(3))*x[0]) + sin((T(3))*x[1])) + (T(2));
      return true;
   }
};

void solveBoneRotations() {
   double x[2];
   x[0] = 5.0;
   x[1] = 1.0;
   ceres::Problem problem;
   ceres::CostFunction* cost_function =
      new ceres::AutoDiffCostFunction<CostFunctor, 1, 2>(new CostFunctor);
   problem.AddResidualBlock(cost_function, NULL, x);

   // Run the solver!
   ceres::Solver::Options options;
   options.minimizer_progress_to_stdout = true;
   ceres::Solver::Summary summary;
   ceres::Solve(options, &problem, &summary);

   std::cout << summary.BriefReport() << "\n";
   std::cout << "x[0], x[1]: " << x[0] << " " << x[1] << "\n";
}