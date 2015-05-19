#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "matrix_math.h"

namespace Geom {

   class Positionalf {
   public:
      inline virtual Eigen::Vector3f getPosition()=0;
      inline virtual void setPosition(Eigen::Vector3f pos)=0;
   };

   class Pointf : public Positionalf {
   public:
      Eigen::Vector3f position;

      Pointf(Eigen::Vector3f pos)
      : position(pos) {}

      inline Eigen::Vector3f getPosition() {
         return position;
      }
      inline void setPosition(Eigen::Vector3f pos) {
         position = pos;
      }
   };

   class Linef {
   public:
      Pointf pntA;
      Pointf pntB;

      Linef(Pointf pA, Pointf pB);
   };

   class Planef {
   public:
      Pointf pntA;
      Pointf pntB;
      Pointf pntC;

      Planef(Pointf pA, Pointf pB, Pointf pC);
   };

}

#endif // __GEOMETRY_H__
