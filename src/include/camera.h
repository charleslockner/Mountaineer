#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "matrix_math.h"

class Camera {
public:
   Eigen::Vector3f position;
   Eigen::Vector3f direction;
   Eigen::Vector3f up;

   float sensitivity;

   Camera(Eigen::Vector3f pos, Eigen::Vector3f dir, Eigen::Vector3f up);
   ~Camera();

   void moveTo(Eigen::Vector3f pos);
   void moveAlong(Eigen::Vector3f dir, float dist);
   void moveRight(float dist);
   void moveLeft(float dist);
   void moveForward(float dist);
   void moveBackward(float dist);
   void moveUp(float dist);
   void moveDown(float dist);
   void lookAt(Eigen::Vector3f pnt);
   void aim(double deltaX, double deltaY);
   Eigen::Matrix4f generateProjViewM();

private:
   void boundPitch();

   Eigen::Matrix4f projectionM;
};

#endif // __CAMERA__
