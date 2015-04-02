
#include "ik_solver.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <assert.h>

class LimbCostFunctor {
public:
   LimbCostFunctor(
      std::vector<short> jointCounts,
      std::vector<Eigen::Matrix4f> boneOffsets,
      std::vector<Eigen::Vector3f> jointAxis
   ) {
      assert(boneOffsets.size() == jointCounts.size());
      this->numBones = jointCounts.size();
      this->totalJointCount = 0;
      for (int i = 0; i < jointCounts.size(); i++)
         this->totalJointCount += jointCounts[i];

      this->jointCounts = jointCounts;
      this->boneOffsets = boneOffsets;
      this->jointAxis = jointAxis;
   }

   template<typename T>
   bool operator()(T const* const* p, T* residuals) const {
      T const* angles = p[0];
      Eigen::Matrix<T,4,4> baseM = Eigen::Map<const Eigen::Matrix<T,4,4> >(p[1], 4, 4);
      Eigen::Matrix<T,3,1> goal = Eigen::Map<const Eigen::Matrix<T,3,1> >(p[2], 3, 1);

      // Start the endEffector off at the origin in model space
      Eigen::Matrix<T,4,1> endEffector = Eigen::Matrix<T,4,1>(T(0),T(0),T(0),T(1));

      // Multiply by all rotation and offset matrices starting at the end and working towards the root
      int jointI = totalJointCount-1;
      for (int i = numBones-1; i >= 0; i--) {
         for (int j = 0; j < jointCounts[i]; j++, jointI--) {
            assert(jointI >= 0);
            T angle = angles[jointI];
            Eigen::Matrix<T,3,1> axis = jointAxis[jointI].cast<T>();
            Eigen::Matrix<T,4,4> jointRotM = Mmath::AngleAxisMatrix(angle, axis);
            endEffector = jointRotM * endEffector;
         }

         Eigen::Matrix<T,4,4> boneOffset = boneOffsets[i].cast<T>();
         endEffector = boneOffset * endEffector;
      }

      // Multiply by model->world matrix and all the bone transforms before this limb (baseM)
      endEffector = baseM * endEffector;

      residuals[0] = goal(0) - endEffector(0);
      residuals[1] = goal(1) - endEffector(1);
      residuals[2] = goal(2) - endEffector(2);
      return true;
   }

private:
   int numBones, totalJointCount;
   std::vector<short> jointCounts;
   std::vector<Eigen::Matrix4f> boneOffsets;
   std::vector<Eigen::Vector3f> jointAxis;
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

   std::vector<Eigen::Matrix4f> boneOffsets;
   std::vector<Eigen::Vector3f> jointAxis;
   std::vector<short> jointCounts;

   // Fill in jointCounts, jointAxis, and boneOffsets from bone data
   for (int i = 0; i < boneCount; i++) {
      Bone * bone = & model->bones[boneIndices[i]];
      int jointCount = bone->joints.size();
      angleCount += jointCount;

      jointCounts.push_back(jointCount);
      boneOffsets.push_back(bone->parentOffset);
      for (int j = 0; j < jointCount; j++)
         jointAxis.push_back(bone->joints[j].axis);
   }

   ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 3> * costFunction =
      new ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 3>(
         new LimbCostFunctor(jointCounts, boneOffsets, jointAxis));

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
   // options.linear_solver_type = ceres::DENSE_QR;
   ceres::Solver::Summary summary;
   ceres::Solve(options, &problem, &summary);

   // Set the calculated angles
   for (int i = 0; i < entAngles.size(); i++)
      *(entAngles[i]) = angleValues[i];
}
