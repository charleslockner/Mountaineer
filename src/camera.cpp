#include "camera.h"

#include <math.h>
#include <stdio.h>
#include <iostream>

#define HFOV   1.0f
#define ASPECT (4.0f/3.0f)
#define NEAR   0.1f
#define FAR    1500.0f

Camera::Camera(Eigen::Vector3f pos, Eigen::Quaternionf rot) :
   Entity(pos, rot), _hfov(HFOV), _aspect(ASPECT), _near(NEAR), _far(FAR) {
}
Camera::Camera(Eigen::Vector3f pos) :
   Entity(pos), _hfov(HFOV), _aspect(ASPECT), _near(NEAR), _far(FAR) {
}

void Camera::aim(double deltaYaw, double deltaPitch, double deltaRoll) {
   rotateAlong(deltaYaw, UP_BASE);
   rotateAlong(deltaPitch, LEFT_BASE);
   rotateAlong(deltaRoll, FORWARD_BASE);
}

void Camera::rigidFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot) {
   if (pos(0) != position(0) || pos(1) != position(1) || pos(2) != position(2)) {
      Eigen::Vector3f dirToPos = (pos - position).normalized();
      float           distTo = (pos - position).norm();

      moveAlong(dirToPos, (distTo < 1 ? 0.05 * distTo : 0.05));
      rotation = rotation.slerp(0.01, rot);
   }
}

void Camera::smoothFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot) {
   if (pos(0) != position(0) || pos(1) != position(1) || pos(2) != position(2)) {
      Eigen::Vector3f dirToPos = (pos - position).normalized();
      float           distance = (pos - position).norm();

      moveAlong(dirToPos, 0.05 * distance);
      rotation = rotation.slerp(0.1, rot);
   }
}

Eigen::Vector3f Camera::rayFromNDCToWorld(float x_nds, float y_nds) {
   Eigen::Matrix4f invPersM = getProjectionM().inverse();
   Eigen::Matrix4f invViewM = getViewM().inverse();

   Eigen::Vector4f ray_clip = Eigen::Vector4f(x_nds, y_nds, -1.0, 1.0);
   Eigen::Vector4f ray_view = invPersM * ray_clip;
                   // unproject the x,y part, so make z component forwards, not a point
                   ray_view = Eigen::Vector4f(ray_view(0), ray_view(1), -1.0, 0.0);
   Eigen::Vector4f ray_world = invViewM * ray_view;

   return ray_world.head<3>().normalized();
}

Eigen::Vector3f Camera::rayFromNDCToView(float x_nds, float y_nds) {
   Eigen::Matrix4f invPersM = getProjectionM().inverse();
   Eigen::Matrix4f invViewM = getViewM().inverse();

   Eigen::Vector4f ray_clip = Eigen::Vector4f(x_nds, y_nds, 1.0, 1.0);
   Eigen::Vector4f ray_view = invPersM * ray_clip;
   return Eigen::Vector3f(ray_view(0), ray_view(1), 1.0).normalized();
}

void Camera::setAspectRatio(float ar) {
   _aspect = ar;
}
void Camera::setHFOV(float hfov) {
   _hfov = hfov;
}
void Camera::setNearDistance(float near) {
   _near = near;
}
void Camera::setFarDistance(float far) {
   _far = far;
}

Eigen::Matrix4f Camera::getViewM() {
   return Mmath::ViewMatrix(position, getForward(), getUp());
}

Eigen::Matrix4f Camera::getProjectionM() {
   return Mmath::PerspectiveMatrix(_hfov, _aspect, _near, _far);
}
