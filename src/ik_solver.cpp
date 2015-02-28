
#include "ik_solver.h"
#include "ceres/ceres.h"

#include <iostream>
#include <stdio.h>

// class CostFunctor {
// public:
//    CostFunctor(Link * root) {
//       this->pTransM[0] = root->parentM;
//       this->pTransM[1] = root->children[0]->parentM;
//       this->pTransM[2] = root->children[0]->children[0]->parentM;
//       this->pTransM[3] = root->children[0]->children[0]->children[0]->parentM;
//       this->pTransM[4] = root->children[0]->children[0]->children[0]->children[0]->parentM;
//    }

//    void setGoal(Eigen::Vector3d goal) {
//       this->goalD = goal;
//    }

//    template <typename T> bool operator()(const T* const angles, T* residual) const {
//       Matrix<T,3,1> goal = goalD.cast<T>();
//       Matrix<T,4,1> startPos = Matrix<T,4,1>(T(1),T(0),T(0),T(1));

//       Matrix<T,4,1> endEffector =
//          pTransM[0].cast<T>() * rotM(angles[0]) *
//          pTransM[1].cast<T>() * rotM(angles[1]) *
//          pTransM[2].cast<T>() * rotM(angles[2]) *
//          pTransM[3].cast<T>() * rotM(angles[3]) *
//          pTransM[4].cast<T>() * rotM(angles[4]) * startPos;

//       Matrix<T,3,1> distVec = goal - endEffector.block(0,0,3,1);
//       residual[0] = distVec(0,0);
//       residual[1] = distVec(1,0);
//       residual[2] = distVec(2,0);

//       return true;
//    }

// private:
//    Eigen::Vector3d goalD;
//    Eigen::Matrix4f pTransM[NUM_BONES];

//    template <typename T> Matrix<T,4,4> rotM(T angle) const {
//       Matrix<T,4,4> m = Matrix<T,4,4>::Identity();
//       Matrix<T,3,3> m3x3;
//       m3x3 = AngleAxis<T>(angle, Matrix<T,3,1>::UnitZ());
//       m.block(0,0,3,3) = m3x3;
//       return m;
//    }
// };

// IKSolver::IKSolver(Link * root) {
//    this->root = root;

//    functor = new CostFunctorDistance(root);
//    ceres::CostFunction * function = new ceres::AutoDiffCostFunction<CostFunctorDistance, 3, NUM_BONES>(functor);

//    problemSimple.AddResidualBlock(function, NULL, angles);
// }

// IKSolver::~IKSolver() {}

// void IKSolver::solveBoneRotations(Eigen::Vector3d goal, SpringState state) {
//    // Set the initial angles
//    this->angles[0] = root->angle;
//    this->angles[1] = root->children[0]->angle;
//    this->angles[2] = root->children[0]->children[0]->angle;
//    this->angles[3] = root->children[0]->children[0]->children[0]->angle;
//    this->angles[4] = root->children[0]->children[0]->children[0]->children[0]->angle;

//    // Select the problem according to key state
//    ceres::Problem * problem = & problemSimple;
//    functorDistance->setGoal(goal);

//    // Run the solver!
//    ceres::Solver::Options options;
//    ceres::Solver::Summary summary;
//    ceres::Solve(options, problem, &summary);

//    // Set the bones to the calculated angles
//    root->angle =                                                     this->angles[0];
//    root->children[0]->angle =                                        this->angles[1];
//    root->children[0]->children[0]->angle =                           this->angles[2];
//    root->children[0]->children[0]->children[0]->angle =              this->angles[3];
//    root->children[0]->children[0]->children[0]->children[0]->angle = this->angles[4];
// }
