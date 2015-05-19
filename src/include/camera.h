#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "entity.h"

class Camera : public Entity {
public:
   Camera(Eigen::Vector3f pos, Eigen::Quaternionf rot);
   Camera(Eigen::Vector3f pos);

   void aim(double deltaYaw, double deltaPitch, double deltaRoll);
   void rigidFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot);
   void smoothFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot);

   void setAspectRatio(float ar);
   void setHFOV(float hfov);
   void setNearDistance(float near);
   void setFarDistance(float far);

   Eigen::Matrix4f getViewM();
   Eigen::Matrix4f getProjectionM();

private:
   float             _hfov;
   float             _aspect;
   float             _near;
   float             _far;

   Eigen::Matrix4f   _viewM;
   Eigen::Matrix4f   _projectionM;
};

#endif // __CAMERA__
