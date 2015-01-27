#include "camera.h"

#include <math.h>
#include <stdio.h>
#include "glm/gtx/rotate_vector.hpp"

#define BASE_SENSITIVITY 0.005
#define PITCH_LIMIT 1.484 // 85 degrees

Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 upVec) {
   position = pos;
   direction = glm::normalize(dir);
   up = glm::normalize(upVec);

   boundPitch();

   sensitivity = 0.5;
}

Camera::~Camera() {}

void Camera::moveTo(glm::vec3 pos) {
   position = pos;
}

void Camera::moveAlong(glm::vec3 dir, float dist) {
   position = position + dist * glm::normalize(dir);
}

void Camera::moveLeft(float dist) {
   moveAlong(glm::normalize(glm::cross(up, direction)), dist);
}

void Camera::moveRight(float dist) {
   moveAlong(-glm::normalize(glm::cross(up, direction)), dist);
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

void Camera::lookAt(glm::vec3 pnt) {
   direction = glm::normalize(pnt - position);
}

void Camera::boundPitch() {
   float pitch = M_PI/2 - acos(glm::dot(up, direction));

   if (pitch > PITCH_LIMIT)
      pitch = PITCH_LIMIT;
   if (pitch < -PITCH_LIMIT)
      pitch = -PITCH_LIMIT;

   direction.y = sin(pitch);
}

void Camera::aim(double deltaX, double deltaY) {
   if (deltaX || deltaY) {
      float pitchDelta = BASE_SENSITIVITY * sensitivity * deltaY;
      float yawDelta = BASE_SENSITIVITY * sensitivity * deltaX;

      glm::vec3 yawApplied = glm::rotate(direction, -yawDelta, up);
      glm::vec3 leftVector = glm::normalize(glm::cross(up, direction));

      direction = glm::rotate(yawApplied, pitchDelta, leftVector);
      boundPitch();
   }
}
