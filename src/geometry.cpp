
#include "geometry.h"

namespace Geom {

   Linef::Linef(Pointf pA, Pointf pB)
   : pntA(pA), pntB(pB) {

   }

   Planef::Planef(Pointf pA, Pointf pB, Pointf pC)
   : pntA(pA), pntB(pB), pntC(pC) {

   }

}