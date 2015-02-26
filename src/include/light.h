#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "matrix_math.h"

#define MAX_LIGHTS 10

// Structure that represents a light
typedef struct {
   Eigen::Vector3f position;
   Eigen::Vector3f direction;
   Eigen::Vector3f color;
   float strength;
   float attenuation;
   float spread; // angle in radians
} Light;

typedef struct {
   Light lights[MAX_LIGHTS];
   unsigned short numLights;
} LightData;

#endif // __LIGHT_H__
