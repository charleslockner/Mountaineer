#ifndef __SPACE_ARRAY_H__
#define __SPACE_ARRAY_H__

#include "geometry.h"

class SpaceArray {
public:
   SpaceArray();
   void Add(Geom::Positionalf * pnt);
   void Remove(Geom::Positionalf * pnt);
   void Resort();
};

#endif // __SPACE_ARRAY_H__