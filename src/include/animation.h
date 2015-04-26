
#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "model.h"

namespace AN {
   Eigen::Matrix4f ComputeKeyframeTransform(AnimBone * animBone, int keyCount, float tickTime, float duration);
}

#endif // __ANIMATION_H__
