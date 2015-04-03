
#include "ik_solver.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <assert.h>

using namespace Eigen;

template<typename T>
Matrix<T,3,1> solveEndEffector(
   Model * model,
   T const* angles,
   int angleCount,
   const std::vector<short>& boneIndices,
   Matrix<T,4,4>& baseM,
   Matrix<T,3,1>& offset
) {
   // Set the point at the offset in the last bone
   Matrix<T,4,1> endEffector = Matrix<T,4,1>(offset(0),offset(1),offset(2),T(1));

   // Multiply by all rotation and offset matrices starting at the end and working towards the root
   int angleI = angleCount-1;
   for (int boneI = boneIndices.size()-1; boneI >= 0; boneI--) {
      Bone * bone = & model->bones[boneIndices[boneI]];
      std::vector<IKJoint> * joints = & bone->joints;

      // Multiply by the rotations from each joint in the bone
      for (int jointI = joints->size()-1; jointI >= 0; jointI--, angleI--) {
         assert(jointI >= 0);
         T angle = angles[angleI];
         Matrix<T,3,1> axis = (*joints)[jointI].axis.cast<T>();
         Matrix<T,4,4> jointRotM = Mmath::AngleAxisMatrix(angle, axis);

         endEffector = jointRotM * endEffector;
      }

      // Multiply by the bone's offset matrix
      Matrix<T,4,4> boneOffset = bone->parentOffset.cast<T>();
      endEffector = boneOffset * endEffector;
   }

   // Multiply by the transforms in the root bone of the limb
   endEffector = baseM * endEffector;

   return Matrix<T,3,1>(endEffector(0), endEffector(1), endEffector(2));
}

class LimbCostFunctor {
private:
   Model * model;
   std::vector<short> boneIndices;
   int numAngles;

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
      Matrix<T,4,4> baseM = Map<const Matrix<T,4,4> >(p[1], 4, 4);
      Matrix<T,3,1> goal = Map<const Matrix<T,3,1> >(p[2], 3, 1);
      Matrix<T,3,1> offset = Matrix<T,3,1>(T(0), T(0), T(0));
      Matrix<T,3,1> endPoint = solveEndEffector(model, angles, numAngles, boneIndices, baseM, offset);

      residuals[0] = goal(0) - endPoint(0);
      residuals[1] = goal(1) - endPoint(1);
      residuals[2] = goal(2) - endPoint(2);
      return true;
   }
};


class RootLimbCostFunctor {
private:
   Model * model;
   std::vector<short> baseBoneIndices;
   std::vector<short> reachBoneIndices;
   int numBaseAngles, numReachAngles;

public:
   RootLimbCostFunctor(
      Model * model,
      std::vector<short> baseBoneIndices,
      std::vector<short> reachBoneIndices,
      int numBaseAngles,
      int numReachAngles
   ) {
      this->model = model;
      this->baseBoneIndices = baseBoneIndices;
      this->reachBoneIndices = reachBoneIndices;
      this->numBaseAngles = numBaseAngles;
      this->numReachAngles = numReachAngles;
   }

   template<typename T>
   bool operator()(T const* const* params, T* residuals) const {
      // Setup variable parameters
      T const* baseAngles = params[0];
      T const* reachAngles = params[1];
      Matrix<T,3,1> modelTrans = Map<const Matrix<T,3,1> >(params[2], 3, 1);
      Matrix<T,4,4> transM = Mmath::TranslationMatrix(modelTrans);

      // Setup constant parameters
      Matrix<T,4,4> modelM = Map<const Matrix<T,4,4> >(params[3], 4, 4);
      Matrix<T,3,1> baseOffset = Map<const Matrix<T,3,1> >(params[4], 3, 1);
      Matrix<T,3,1> reachOffset = Map<const Matrix<T,3,1> >(params[5], 3, 1);
      Matrix<T,3,1> baseGoal = Map<const Matrix<T,3,1> >(params[6], 3, 1);
      Matrix<T,3,1> reachGoal = Map<const Matrix<T,3,1> >(params[7], 3, 1);

      // Multiply by the variable transM (the root bone can move around in world space)
      Matrix<T,4,4> baseM = modelM * transM;

      // Solve for the end effectors from the base and reach limbs
      Matrix<T,3,1> basePoint = solveEndEffector(model, baseAngles, numBaseAngles,
                                                 baseBoneIndices, baseM, baseOffset);
      Matrix<T,3,1> reachPoint = solveEndEffector(model, reachAngles, numReachAngles,
                                                  reachBoneIndices, baseM, reachOffset);

      // minimize distances to end effectors from goals
      residuals[0] = baseGoal(0) - basePoint(0);
      residuals[1] = baseGoal(1) - basePoint(1);
      residuals[2] = baseGoal(2) - basePoint(2);
      residuals[3] = reachGoal(0) - reachPoint(0);
      residuals[4] = reachGoal(1) - reachPoint(1);
      residuals[5] = reachGoal(2) - reachPoint(2);
      return true;
   }
};

namespace IK {

   void SolveSegment(
      Model * model,
      Matrix4f baseM,
      Vector3f goal,
      std::vector<float *> entAngles,
      std::vector<short> boneIndices
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
      Matrix4d baseMValues = baseM.cast<double>();
      Vector3d goalValues = goal.cast<double>();

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

      // Set everything constant except for the endAngles
      problem.SetParameterBlockVariable(angleData);
      problem.SetParameterBlockConstant(baseMData);
      problem.SetParameterBlockConstant(goalData);

      // Solve for the endAngles
      ceres::Solver::Options options;
      options.linear_solver_type = ceres::DENSE_QR;
      ceres::Solver::Summary summary;
      ceres::Solve(options, &problem, &summary);

      // Set the calculated angles
      for (int i = 0; i < entAngles.size(); i++)
         *(entAngles[i]) = angleValues[i];
   }

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
   ) {

   }
}
