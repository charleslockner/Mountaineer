#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "matrix_math.h"

class GPoint {
public:
   virtual Eigen::Vector3f getPosition()=0;
};



#endif // __GEOMETRY_H__
