#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "entity.h"

// NDC = Normalized Device Coordinates

class Camera : public Entity {
public:
   Camera(Eigen::Vector3f pos, Eigen::Quaternionf rot);
   Camera(Eigen::Vector3f pos);

   void aim(double deltaYaw, double deltaPitch, double deltaRoll);
   void rigidFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot);
   void smoothFollow(Eigen::Vector3f pos, Eigen::Quaternionf rot);

   // Returns a normalized vector in world coordinates
   // pointing towards the x,y coordinate specified on the screen
   // x_nds and y_nds should be in the range [-1, 1]
   Eigen::Vector3f rayFromNDCToWorld(float x_nds, float y_nds);
   Eigen::Vector3f rayFromNDCToView(float x_nds, float y_nds);

   void setViewFrustum();
   void setAspectRatio(float aspect);
   void setFOVY(float fovy);
   void setNearDistance(float near);
   void setFarDistance(float far);

   Eigen::Matrix4f getViewM();
   Eigen::Matrix4f getProjectionM();

private:
   float             _fovy;
   float             _aspect;
   float             _nearDist;
   float             _farDist;

   // Frustum in camera space
   Geom::Frustumf    _viewFrustum;

   Eigen::Matrix4f   _viewM;
   Eigen::Matrix4f   _projectionM;
};

#endif // __CAMERA__
