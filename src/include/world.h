#ifndef __WORLD_H__
#define __WORLD_H__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#define MAX_LIGHTS 10

// Structure that represents a light
typedef struct {
   glm::vec3 position;
   glm::vec3 direction;
   glm::vec3 color;
   float strength;
   float attenuation;
   float spread; // angle in radians
} Light;

typedef struct {
   Light lights[MAX_LIGHTS];
   unsigned short numLights;
} World;

#endif // __WORLD_H__