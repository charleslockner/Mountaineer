
#include "ik_solver.h"
#include "ceres/ceres.h"

#include <iostream>
#include <vector>
#include <stdio.h>

struct CostFunctor {
   template<typename T>
   bool operator()(T const* const* parameters, T* residuals) const {
      T const* angles = parameters[0];
      T const* axisValues = parameters[1];
      T const* goal = parameters[2];
   }
};

IKSolver::IKSolver(Model * model) {
   this->model = model;
}

IKSolver::~IKSolver() {}

static void fillBoneOffsets(Model * model, IKLimb * limb, std::vector<Eigen::Matrix4f*>& boneOffsets) {
   for (int i = 0; i < limb->joints.size(); i++) {
      int jointBoneIndex = limb->joints[i].boneIndex;
      Eigen::Matrix4f * boneOffset = &(model->bones[jointBoneIndex].parentOffset);
      boneOffsets.push_back(boneOffset);
   }
}

void IKSolver::solveBoneRotations(IKLimb * limb, float * angles) {
   std::vector<Eigen::Matrix4f*> boneOffsets;
   fillBoneOffsets(model, limb, boneOffsets);

   // ceres::DynamicAutoDiffCostFunction<CostFunctor, 4> * function =
   //    new DynamicAutoDiffCostFunction<CostFunctor, 4>(new CostFunctor());
   // function->AddParameterBlock(5);
   // function->AddParameterBlock(10);
   // function->SetNumResiduals(3);

   // problem.AddResidualBlock(function, NULL, angles);
   // // problem.SetParameterLowerBound(double* values, int index, double lower_bound);
   // // problem.SetParameterUpperBound(double* values, int index, double upper_bound);

   // ceres::Solver::Options options;
   // ceres::Solver::Summary summary;
   // ceres::Solve(options, problem, &summary);

//    // Set the bones to the calculated angles
//    root->angle =                                                     this->angles[0];
//    root->children[0]->angle =                                        this->angles[1];
//    root->children[0]->children[0]->angle =                           this->angles[2];
//    root->children[0]->children[0]->children[0]->angle =              this->angles[3];
//    root->children[0]->children[0]->children[0]->children[0]->angle = this->angles[4];
}
