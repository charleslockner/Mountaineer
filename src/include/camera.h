#ifndef __CAMERA__
#define __CAMERA__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"


class Camera {
public:
   glm::vec3 position;
   glm::vec3 direction;
   glm::vec3 up;

   float sensitivity;

   Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up);
   ~Camera();

   void moveTo(glm::vec3 pos);
   void moveAlong(glm::vec3 dir, float dist);
   void moveRight(float dist);
   void moveLeft(float dist);
   void moveForward(float dist);
   void moveBackward(float dist);
   void moveUp(float dist);
   void moveDown(float dist);
   void lookAt(glm::vec3 pnt);
   void aim(double deltaX, double deltaY);

private:
   void boundPitch();
};

#endif // __CAMERA__