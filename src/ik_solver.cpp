
#include "ik_solver.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <assert.h>

class LimbCostFunctor {
public:
   LimbCostFunctor(
      std::vector<int> jointCounts,
      std::vector<Eigen::Matrix4f*> boneOffsets,
      std::vector<Eigen::Vector3f*> jointAxis,
      Eigen::Matrix4f* invBindPose
   ) {
      assert(boneOffsets.size() == jointCounts.size());
      int numBones = jointCounts.size();
      this->jointCounts = jointCounts;
      this->boneOffsets = boneOffsets;
      this->jointAxis = jointAxis;
      this->invBindPose = invBindPose;
      this->totalJointCount = 0;
      for (int i = 0; i < jointCounts.size(); i++)
         this->totalJointCount += jointCounts[i];
   }

   template<typename T>
   bool operator()(T const* const* p, T* residuals) const {
      T const* angles = p[0];
      Eigen::Matrix<T,4,4> baseTransform = Eigen::Map<const Eigen::Matrix<T,4,4> >(p[1], 4, 4);
      Eigen::Matrix<T,3,1> goal = Eigen::Map<const Eigen::Matrix<T,3,1> >(p[2], 3, 1);

      // Start the endEffector off at the origin in model space (multiply by bone->model matrix)
      Eigen::Matrix<T,4,1> endEffector = Eigen::Matrix<T,4,1>(T(0),T(0),T(0),T(0));
      endEffector = invBindPose->cast<T>() * endEffector;

      int jointI = totalJointCount-1;
      // Multiply by all rotation and offset matrices starting at the end and working towards the root
      for (int i = numBones-1; i >= 0; i--) {
         for (int j = 0; j < jointCounts[i]; j++, jointI--) {
            assert(jointI >= 0);
            T angle = angles[jointI];
            Eigen::Matrix<T,3,1> axis = (*jointAxis[jointI]).cast<T>();
            Eigen::Matrix<T,4,4> jointRotM = Mmath::angleAxisMatrix(angle, axis);
            endEffector = jointRotM * endEffector;
         }

         Eigen::Matrix<T,4,4> boneOffset = (*boneOffsets[i]).cast<T>();
         endEffector = boneOffset * endEffector;
      }

      // Multiply by model->world matrix and all the bone transforms before this limb,
      // combined into 1 matrix: baseTransform
      endEffector = baseTransform * endEffector;

      residuals[0] = goal(0) - endEffector(0);
      residuals[1] = goal(1) - endEffector(1);
      residuals[2] = goal(2) - endEffector(2);
      return true;
   }

private:
   int numBones, totalJointCount;
   std::vector<int> jointCounts;
   std::vector<Eigen::Matrix4f*> boneOffsets;
   std::vector<Eigen::Vector3f*> jointAxis;
   Eigen::Matrix4f* invBindPose;
};

IKSolver::IKSolver(Model * model) {
   this->model = model;
   std::vector<int> jointCounts;
   std::vector<Eigen::Matrix4f*> boneOffsets;
   std::vector<Eigen::Vector3f*> jointAxis;
   Eigen::Matrix4f* invBindPose;

   costFunction = new ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 4>(
      new LimbCostFunctor(jointCounts, boneOffsets, jointAxis, invBindPose)
   );
}

IKSolver::~IKSolver() {}

void IKSolver::solveBoneRotations(IKLimb * limb, float * angles) {

   // std::vector<double> angleData;
   // std::vector<double> offsets;
   // std::vector<int> jointCounts;
   // std::vector<double> axisValues;
   // int numAngles = 0;
   // double * dataPtr;

   // for (int i = 0; i < limb->boneIndices.size(); i++) {
   //    Bone * bone = & model->bones[limb->boneIndices[i]];

   //    // add parentOffsets
   //    Eigen::Matrix4d offsetD = bone->parentOffset.cast<double>();
   //    dataPtr = offsetD.data();
   //    offsets.insert(offsets.end(), dataPtr, & dataPtr[16]);

   //    // add jointCounts
   //    jointCounts.push_back(bone->jointCount);

   //    // add other stuff
   //    for (int j = 0; j < bone->jointCount; j++) {
   //       Eigen::Vector3d axisD = bone->joints[j].axis.cast<double>();
   //       dataPtr = axisD.data();
   //       axisValues.insert(axisValues.end(), dataPtr, & dataPtr[3]);

   //       numAngles++;
   //    }
   // }

   // // add angles
   // for (int i = 0; i < numAngles; i++)
   //    angleData.push_back(static_cast<double>(angles[i]));

   // function->AddParameterBlock(5);
   // function->AddParameterBlock(10);
   // function->SetNumResiduals(3);

   // problem.AddResidualBlock(function, NULL, angles);
   // problem.SetParameterLowerBound(double* values, int index, double lower_bound);
   // problem.SetParameterUpperBound(double* values, int index, double upper_bound);

   // ceres::Solver::Options options;
   // ceres::Solver::Summary summary;
   // ceres::Solve(options, problem, &summary);

   // angles is set from the ceres::Solve, so theres nothing more to do
}
