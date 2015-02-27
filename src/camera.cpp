#include "camera.h"

#include <math.h>
#include <stdio.h>
#include <iostream>

#define BASE_SENSITIVITY 0.005
#define PITCH_LIMIT 1.484 // 85 degrees

#define PI 3.1415927f

#define HFOV   1.0f //PI/4.0f
#define ASPECT 4.0f / 3.0f
#define NEAR   0.1f
#define FAR    1000.0f

Camera::Camera(Eigen::Vector3f pos, Eigen::Vector3f dir, Eigen::Vector3f upVec) {
   position = pos;
   direction = dir.normalized();
   up = upVec.normalized();

   boundPitch();

   sensitivity = 0.5;

   projectionM = Mmath::perspectiveMatrix(HFOV, ASPECT, NEAR, FAR);
}

Camera::~Camera() {}

void Camera::moveTo(Eigen::Vector3f pos) {
   position = pos;
}

void Camera::moveAlong(Eigen::Vector3f dir, float dist) {
   position = position + dist * dir.normalized();
}

void Camera::moveLeft(float dist) {
   moveAlong(up.cross(direction).normalized(), dist);
}

void Camera::moveRight(float dist) {
   moveAlong(- up.cross(direction).normalized(), dist);
}

void Camera::moveForward(float dist) {
   moveAlong(direction, dist);
}

void Camera::moveBackward(float dist) {
   moveAlong(-direction, dist);
}

void Camera::moveUp(float dist) {
   moveAlong(up, dist);
}

void Camera::moveDown(float dist) {
   moveAlong(-up, dist);
}

void Camera::lookAt(Eigen::Vector3f pnt) {
   Eigen::Vector3f dir = pnt - position;
   direction = dir.normalized();
}

void Camera::boundPitch() {
   float pitch = M_PI/2 - acos(up.dot(direction));

   if (pitch > PITCH_LIMIT)
      pitch = PITCH_LIMIT;
   if (pitch < -PITCH_LIMIT)
      pitch = -PITCH_LIMIT;

   direction.y() = sin(pitch);
}

void Camera::aim(double deltaX, double deltaY) {
   if (deltaX || deltaY) {
      float pitchDelta = BASE_SENSITIVITY * sensitivity * deltaY;
      float yawDelta = BASE_SENSITIVITY * sensitivity * deltaX;

      Eigen::Vector3f yawApplied = Mmath::rotateVec3(direction, -yawDelta, up);
      Eigen::Vector3f leftVector = up.cross(direction).normalized();

      direction = Mmath::rotateVec3(yawApplied, pitchDelta, leftVector);
      boundPitch();
   }
}

Eigen::Matrix4f Camera::getViewM() {
   return Mmath::lookAtMatrix(position, direction, up);
}

Eigen::Matrix4f Camera::getProjectionM() {
   return projectionM;
}
