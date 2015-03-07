#ifndef __IK_SOLVER_H__
#define __IK_SOLVER_H__

#include "matrix_math.h"
#include "model.h"
#include <vector>

class IKSolver {
public:
   IKSolver(Model * model);
   ~IKSolver();
   void solveBoneRotations(IKLimb * limb, float * angles);

private:
   Model * model;
};

#endif // __IK_SOLVER_H__
