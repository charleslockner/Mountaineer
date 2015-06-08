
#include "entity.h"
#include "animation.h"
#include "matrix_math.h"
#include "ceres/ceres.h"

#include <vector>
#include <assert.h>

using namespace Eigen;

struct Model;

template<typename T>
Matrix<T,3,1> solveEndEffector(
   Model * model,
   T const* angles,
   int angleCount,
   const std::vector<int>& boneIndices,
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
   std::vector<int> boneIndices;
   Matrix4f baseM;
   Eigen::Vector3f goal;
   int numAngles;

public:
   LimbCostFunctor(
      Model * model,
      std::vector<int>& boneIndices,
      Matrix4f baseM,
      Vector3f goal,
      int numAngles
   ) {
      this->model = model;
      this->boneIndices = boneIndices;
      this->baseM = baseM;
      this->goal = goal;
      this->numAngles = numAngles;
   }

   template<typename T>
   bool operator()(T const* const* params, T* residuals) const {
      T const* angles = params[0];
      Matrix<T,4,4> baseM_T = baseM.cast<T>();
      Matrix<T,3,1> goalT = goal.cast<T>();
      Matrix<T,3,1> offset = Matrix<T,3,1>(T(0), T(0), T(0));
      Matrix<T,3,1> endPoint = solveEndEffector(model, angles, numAngles, boneIndices, baseM_T, offset);

      residuals[0] = goalT(0) - endPoint(0);
      residuals[1] = goalT(1) - endPoint(1);
      residuals[2] = goalT(2) - endPoint(2);
      return true;
   }
};

class BaseLimbCostFunctor {
private:
   Model * model;
   std::vector<int> boneIndices;
   Matrix4f scaleParentM;
   Eigen::Vector3f goal;
   int numAngles;

public:
   BaseLimbCostFunctor(
      Model * model,
      std::vector<int>& boneIndices,
      Matrix4f scaleParentM,
      Vector3f goal,
      int numAngles
   ) {
      this->model = model;
      this->boneIndices = boneIndices;
      this->scaleParentM = scaleParentM;
      this->goal = goal;
      this->numAngles = numAngles;
   }

   template<typename T>
   bool operator()(T const* const* params, T* residuals) const {
      T const* angles = params[0];
      Matrix<T,3,1> worldPos = Map<const Matrix<T,3,1> >(params[1], 3, 1);
      Quaternion<T> worldRot = Map<const Quaternion<T> >(params[2]);
      Matrix<T,4,4> baseM = Mmath::TranslationMatrix(worldPos) *
                            Mmath::RotationMatrix(worldRot) *
                            scaleParentM.cast<T>();
      Matrix<T,3,1> goalT = goal.cast<T>();
      Matrix<T,3,1> offset = Matrix<T,3,1>(T(0), T(0), T(0));
      Matrix<T,3,1> endPoint = solveEndEffector(model, angles, numAngles, boneIndices, baseM, offset);

      // Should change this so we get the endPoint first and then move the root so that
      // the end point is constant no matter what
      residuals[0] = goalT(0) - endPoint(0);
      residuals[1] = goalT(1) - endPoint(1);
      residuals[2] = goalT(2) - endPoint(2);
      return true;
   }
};

void IKEntity::solveLimbs(Eigen::Matrix4f parentM, std::vector<IKLimb *> limbs) {
   ceres::Problem problem;

   // Setup the position as a double vec3
   Vector3d positionD = position.cast<double>();
   Quaterniond rotationD = rotation.cast<double>();
   double * posData = positionD.data();
   double * rotData = (double *)(& rotationD);

   std::vector<std::vector<double> > angleValuesByLimb = std::vector<std::vector<double> >(limbs.size());

   for (int limbNum = 0; limbNum < limbs.size(); limbNum++) {
      IKLimb * limb = limbs[limbNum];
      angleValuesByLimb[limbNum] = std::vector<double>(0);

      // Quick reference for angle and bone counts
      int angleCount = limb->jointAngles.size();
      int boneCount = limb->boneIndices.size();

      // Set up the angles data ptr
      std::vector<double>* angleValues = & angleValuesByLimb[limbNum];
      for (int i = 0; i < limb->jointAngles.size(); i++)
         angleValues->push_back(double(*(limb->jointAngles[i])));
      double * angleData = angleValues->data();

      // Create the cost functor and problem
      if (limb->isBase) {
         // Create the rotScaleParent part of the base matrix
         Matrix4f sclM = Mmath::ScaleMatrix(scale);
         Matrix4f scaleParentM = sclM * parentM;

         // Create the Cost function
         ceres::DynamicAutoDiffCostFunction<BaseLimbCostFunctor, 4> * costFunction =
            new ceres::DynamicAutoDiffCostFunction<BaseLimbCostFunctor, 4>(
               new BaseLimbCostFunctor(this->model, limb->boneIndices, scaleParentM, limb->goal, angleCount));

         // Set up the cost function parameter and residual blocks
         costFunction->AddParameterBlock(angleCount); // angles
         costFunction->AddParameterBlock(3);          // position
         costFunction->AddParameterBlock(4);          // rotation
         costFunction->SetNumResiduals(3);            // residuals
         problem.AddResidualBlock(costFunction, NULL, angleData, posData, rotData);

      } else {
         // Set up the base matrix
         Matrix4f modelM = Mmath::TransformationMatrix(position, rotation, scale);
         Matrix4f baseM = modelM * parentM;

         // Create the Cost function
         ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 4> * costFunction =
            new ceres::DynamicAutoDiffCostFunction<LimbCostFunctor, 4>(
               new LimbCostFunctor(this->model, limb->boneIndices, baseM, limb->goal, angleCount));

         // Set up the cost function parameter and residual blocks
         costFunction->AddParameterBlock(angleCount); // angles
         costFunction->SetNumResiduals(3);            // residuals
         problem.AddResidualBlock(costFunction, NULL, angleData);
      }

      // set min and max values of each angle
      int angleIndex = 0;
      for (int i = 0; i < boneCount; i++) {
         Bone * bone = & model->bones[limb->boneIndices[i]];
         for (int j = 0; j < bone->joints.size(); j++) {
            problem.SetParameterLowerBound(angleData, angleIndex, bone->joints[j].minAngle);
            problem.SetParameterUpperBound(angleData, angleIndex, bone->joints[j].maxAngle);
            angleIndex++;
         }
      }
   }

   // Solve for the angles
   ceres::Solver::Options options;
   options.linear_solver_type = ceres::DENSE_QR;
   ceres::Solver::Summary summary;
   ceres::Solve(options, &problem, &summary);

   // Set the calculated angles
   for (int limbNum = 0; limbNum < limbs.size(); limbNum++) {
      IKLimb * limb = limbs[limbNum];
      std::vector<double>* angleValues = & angleValuesByLimb[limbNum];

      for (int i = 0; i < limb->jointAngles.size(); i++)
         *(limb->jointAngles[i]) = (*angleValues)[i];
   }

   // Update the entity's position in case a base limb was used
   position = positionD.cast<float>();
   rotation = rotationD.cast<float>();
}


// ------------------------ IK Bone ------------------------ //
IKLimb::IKLimb() {}
IKLimb::~IKLimb() {}
// --------------------------------------------------------- //
// ======================= IK Entity ======================= //
// --------------------------------------------------------- //
IKEntity::IKEntity(Eigen::Vector3f pos, Model * model)
: BonifiedEntity(pos, model) {
   this->ikLimbs = std::vector<IKLimb *>(0);
   this->ikBones = std::vector<IKBone>(model->boneCount);
   for (int i = 0; i < model->boneCount; i++) {
      this->ikBones[i].angles = std::vector<double>(model->bones[i].joints.size());
      this->ikBones[i].limbs = std::vector<IKLimb *>(0);
   }
   usingIK = false;
}

void IKEntity::addLimb(std::vector<int> boneIndices, Eigen::Vector3f offset, bool isBase) {
   assert(boneIndices.size() > 0);

   IKLimb * limb = new IKLimb();
   limb->isBase = isBase;
   limb->boneIndices = boneIndices;
   limb->offset = offset;
   limb->goal = Eigen::Vector3f(0,0,0);
   limb->jointAngles = constructJointAnglePtrs(boneIndices);
   this->ikBones[boneIndices[0]].limbs.push_back(limb);
   this->ikLimbs.push_back(limb);
}

void IKEntity::setLimbGoal(int limbIndex, Eigen::Vector3f goal) {
   this->ikLimbs[limbIndex]->goal = goal;
}

void IKEntity::update(float tickDelta) {
   if (model->hasBoneTree && model->hasAnimations) {
      BonifiedEntity::replayIfNeeded(tickDelta);
      computeAnimMs(model->boneRoot, Eigen::Matrix4f::Identity());
   }
}

void IKEntity::animateWithIK() {
   usingIK = true;
}

void IKEntity::animateWithKeyframes() {
   usingIK = false;
}

void IKEntity::computeAnimMs(int boneIndex, Eigen::Matrix4f parentM) {
   int animNum = this->animNums[boneIndex];
   float tickTime = this->animTimes[boneIndex];

   Bone * bone = & model->bones[boneIndex];
   IKBone * ikBone = & this->ikBones[boneIndex];
   Animation * anim = & model->animations[animNum];
   AnimBone * animBone = & anim->animBones[boneIndex];

   // Compute rotation angles for all limbs who's root starts at this bone
   if (usingIK && ikBone->limbs.size())
      solveLimbs(parentM, ikBone->limbs);

   // If this bone has a computed ik rotation, use it, otherwise use the animation rotation
   Eigen::Matrix4f animM = (usingIK) ? (
      (bone->joints.size() > 0) ?
         animM = constructJointMatrix(boneIndex) :
         animM = bone->parentOffset) :
      AN::ComputeKeyframeTransform(animBone, anim->keyCount, tickTime, anim->duration);

   boneMs[boneIndex] = parentM * animM;
   animMs[boneIndex] = boneMs[boneIndex] * bone->invBonePose;

   for (int i = 0; i < bone->childCount; i++)
      computeAnimMs(bone->childIndices[i], boneMs[boneIndex]);
}

std::vector<double *> IKEntity::constructJointAnglePtrs(std::vector<int>& boneIndices) {
   std::vector<double *> jointAngles = std::vector<double *>();
   for (int i = 0; i < boneIndices.size(); i++) {
      IKBone * ikBone = & ikBones[boneIndices[i]];
      for (int j = 0; j < ikBone->angles.size(); j++)
         jointAngles.push_back(& ikBone->angles[j]);
   }
   return jointAngles;
}

Eigen::Matrix4f IKEntity::constructJointMatrix(int boneIndex) {
   Bone * bone = & model->bones[boneIndex];
   IKBone * ikBone = & ikBones[boneIndex];

   Eigen::Matrix4f jointRotationM = Eigen::Matrix4f::Identity();
   for (int i = 0; i < bone->joints.size(); i++) {
      assert(bone->joints.size() == ikBone->angles.size());
      jointRotationM *= Mmath::AngleAxisMatrix<float>(ikBone->angles[i], bone->joints[i].axis);
   }

   return bone->parentOffset * jointRotationM;
}

