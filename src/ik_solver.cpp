
#include "ik_solver.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <assert.h>

class LimbCostFunctor {
public:
   LimbCostFunctor(
      Model * model,
      std::vector<short> boneIndices,
      int numAngles
   ) {
      this->model = model;
      this->boneIndices = boneIndices;
      this->numAngles = numAngles;
   }

   template<typename T>
   bool operator()(T const* const* p, T* residuals) const {
      T const* angles = p[0];
      Eigen::Matrix<T,4,4> baseM = Eigen::Map<const Eigen::Matrix<T,4,4> >(p[1], 4, 4);
      Eigen::Matrix<T,3,1> goal = Eigen::Map<const Eigen::Matrix<T,3,1> >(p[2], 3, 1);

      // Start the endPoint off at its position relative to the location of the last bone
      Eigen::Matrix<T,4,1> endPoint = Eigen::Matrix<T,4,1>(T(0),T(0),T(0),T(1));

      // Multiply by all rotation and offset matrices starting at the end and working towards the root
      int angleI = numAngles-1;
      for (int boneI = boneIndices.size()-1; boneI >= 0; boneI--) {
         Bone * bone = & model->bones[boneIndices[boneI]];
         std::vector<IKJoint> * joints = & bone->joints;

         for (int jointI = joints->size()-1; jointI >= 0; jointI--, angleI--) {
            assert(jointI >= 0);
            T angle = angles[angleI];
            Eigen::Matrix<T,3,1> axis = (*joints)[jointI].axis.cast<T>();
            Eigen::Matrix<T,4,4> jointRotM = Mmath::AngleAxisMatrix(angle, axis);
            endPoint = jointRotM * endPoint;
         }

         Eigen::Matrix<T,4,4> boneOffset = bone->parentOffset.cast<T>();
         endPoint = boneOffset * endPoint;
      }

      // Multiply by model->world matrix and all the bone transforms before this limb (baseM)
      endPoint = baseM * endPoint;

      residuals[0] = goal(0) - endPoint(0);
      residuals[1] = goal(1) - endPoint(1);
      residuals[2] = goal(2) - endPoint(2);
      return true;
   }

private:
   Model * model;
   std::vector<short> boneIndices;
   int numAngles;
};

IKSolver::IKSolver(Model * model, std::vector<short> boneIndices) {
   this->model = model;
   this->boneIndices = boneIndices;
}

void IKSolver::solveBoneRotations(
   Eigen::Matrix4f baseM,
   Eigen::Vector3f goal,
   std::vector<float *> entAngles
) {
   int angleCount = 0;
   int boneCount = boneIndices.size();
   for (int i = 0; i < boneCount; i++)
      angleCount += model->bones[boneIndices[i]].joints.size();

   // Create the cost functor and pass in the constant arguments
   ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 3> * costFunction =
      new ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 3>(
         new LimbCostFunctor(model, boneIndices, angleCount));

   // Set up the cost function parameter and residual blocks
   costFunction->AddParameterBlock(angleCount); // angles
   costFunction->AddParameterBlock(16);         // baseMatrix
   costFunction->AddParameterBlock(3);          // goal
   costFunction->SetNumResiduals(3);            // residuals

   // Setup up the parameter data blocks
   std::vector<double> angleValues = std::vector<double>();
   for (int i = 0; i < entAngles.size(); i++)
      angleValues.push_back(double(*(entAngles[i])));
   Eigen::Matrix4d baseMValues = baseM.cast<double>();
   Eigen::Vector3d goalValues = goal.cast<double>();

   assert(entAngles.size() == angleCount);

   double * angleData = angleValues.data();
   double * baseMData = baseMValues.data();
   double * goalData = goalValues.data();

   ceres::Problem problem;
   problem.AddParameterBlock(angleData, angleCount);
   problem.AddParameterBlock(baseMData, 16);
   problem.AddParameterBlock(goalData, 3);
   problem.AddResidualBlock(costFunction, NULL, angleData, baseMData, goalData);

   // set min and max values of each angle
   int angleIndex = 0;
   for (int i = 0; i < boneCount; i++) {
      Bone * bone = & model->bones[boneIndices[i]];
      for (int j = 0; j < bone->joints.size(); j++) {
         problem.SetParameterLowerBound(angleData, angleIndex, bone->joints[j].minAngle);
         problem.SetParameterUpperBound(angleData, angleIndex, bone->joints[j].maxAngle);
         angleIndex++;
      }
   }

   // Set everything constant except for the angles
   problem.SetParameterBlockVariable(angleData);
   problem.SetParameterBlockConstant(baseMData);
   problem.SetParameterBlockConstant(goalData);

   // Solve for the angles
   ceres::Solver::Options options;
   options.linear_solver_type = ceres::DENSE_QR;
   ceres::Solver::Summary summary;
   ceres::Solve(options, &problem, &summary);

   // Set the calculated angles
   for (int i = 0; i < entAngles.size(); i++)
      *(entAngles[i]) = angleValues[i];
}
