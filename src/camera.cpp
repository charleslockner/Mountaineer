#include "camera.h"

#define FOVY   1.0f
#define ASPECT (4.0f/3.0f)
#define NEAR   0.1f
#define FAR    1500.0f

Camera::Camera(Eigen::Vector3f pos, Eigen::Quaternionf rot) :
   Entity(pos, rot), _fovy(FOVY), _aspect(ASPECT), _nearDist(NEAR), _farDist(FAR) {
}
Camera::Camera(Eigen::Vector3f pos) :
   Entity(pos), _fovy(FOVY), _aspect(ASPECT), _nearDist(NEAR), _farDist(FAR) {
}

void Camera::aim(double deltaYaw, double deltaPitch, double deltaRoll) {
   rotateAlong(deltaYaw, UP_BASE);
   rotateAlong(deltaPitch, LEFT_BASE);
   rotateAlong(deltaRoll, FORWARD_BASE);
}

void Camera::rigidFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot) {
   if (pos(0) != position(0) || pos(1) != position(1) || pos(2) != position(2)) {
      Eigen::Vector3f dirToPos = (pos - position).normalized();
      float           distance = (pos - position).norm();

      moveAlong(dirToPos, (distance < 1 ? distance : 0.2 * distance));
      rotation = rotation.slerp(0.1, rot);
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
   Eigen::Vector4f ray_clip = Eigen::Vector4f(x_nds, y_nds, 1.0, 1.0);
   Eigen::Vector4f ray_view = getProjectionM().inverse() * ray_clip;
                   // unproject the x,y part, so make z component forwards, not a point
                   ray_view = Eigen::Vector4f(ray_view(0), ray_view(1), -1.0, 0.0);
   Eigen::Vector4f ray_world = getViewM().inverse() * ray_view;
   return ray_world.head<3>().normalized();
}

Eigen::Vector3f Camera::rayFromNDCToView(float x_nds, float y_nds) {
   Eigen::Vector4f ray_clip = Eigen::Vector4f(x_nds, y_nds, 1.0, 1.0);
   Eigen::Vector4f ray_view = getProjectionM().inverse() * ray_clip;
   return Eigen::Vector3f(ray_view(0), ray_view(1), 1.0).normalized();
}

void Camera::setViewFrustum() {
   float halfNearH = tan(_fovy / 2) * _nearDist;
   float halfNearW = halfNearH * _aspect;
   float halfFarH  = tan(_fovy / 2) * _farDist;
   float halfFarW  = halfFarH * _aspect;

   Eigen::Vector3f nbl = Eigen::Vector3f(-halfNearW, -halfNearH, _nearDist);
   Eigen::Vector3f nbr = Eigen::Vector3f( halfNearW, -halfNearH, _nearDist);
   Eigen::Vector3f ntl = Eigen::Vector3f(-halfNearW,  halfNearH, _nearDist);
   Eigen::Vector3f ntr = Eigen::Vector3f( halfNearW,  halfNearH, _nearDist);
   Eigen::Vector3f fbl = Eigen::Vector3f(-halfFarW, -halfFarH, _farDist);
   Eigen::Vector3f fbr = Eigen::Vector3f( halfFarW, -halfFarH, _farDist);
   Eigen::Vector3f ftl = Eigen::Vector3f(-halfFarW,  halfFarH, _farDist);
   Eigen::Vector3f ftr = Eigen::Vector3f( halfFarW,  halfFarH, _farDist);

   Geom::Planef leftP   = Geom::Planef(ftl, ntl, nbl);
   Geom::Planef rightP  = Geom::Planef(ntr, ftr, fbr);
   Geom::Planef bottomP = Geom::Planef(fbr, fbl, nbl);
   Geom::Planef topP    = Geom::Planef(ntr, ntl, ftl);
   Geom::Planef nearP   = Geom::Planef(nbl, FORWARD_BASE);
   Geom::Planef farP    = Geom::Planef(fbl, -FORWARD_BASE);

   _viewFrustum = Geom::Frustumf(leftP, rightP, bottomP, topP, nearP, farP);
}

void Camera::setAspectRatio(float ar) {
   _aspect = ar;
}
void Camera::setFOVY(float fovy) {
   _fovy = fovy;
}
void Camera::setNearDistance(float near) {
   _nearDist = near;
}
void Camera::setFarDistance(float far) {
   _farDist = far;
}

Eigen::Matrix4f Camera::getViewM() {
   return Mmath::ViewMatrix(position, getForward(), getUp());
}

Eigen::Matrix4f Camera::getProjectionM() {
   return Mmath::PerspectiveMatrix(_fovy, _aspect, _nearDist, _farDist);
}
