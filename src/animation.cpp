
#include "animation.h"

#include "matrix_math.h"

static Eigen::Vector3f lerpVec3(Eigen::Vector3f begin, Eigen::Vector3f end, float ratio) {
   return begin + ratio * (end - begin);
}

static int findEarlyKeyIndex(int keyCount, float tickTime, float duration) {
   int index = tickTime * (keyCount-1) / duration;
   return index < keyCount-1 ? index : keyCount - 2;
}

static Key interpolateKeys(Key earlyKey, Key lateKey, float tickTime) {
   Key key;
   tickTime = fmax(earlyKey.time, fmin(lateKey.time, tickTime));
   float ratio = (tickTime - earlyKey.time) / (lateKey.time - earlyKey.time);

   key.position = lerpVec3(earlyKey.position, lateKey.position, ratio);
   key.rotation = earlyKey.rotation.slerp(ratio, lateKey.rotation);
   key.scale = lerpVec3(earlyKey.scale, lateKey.scale, ratio);
   return key;
}

namespace AN {

   // Assuming keyCount >= 1
   Eigen::Matrix4f ComputeKeyframeTransform(AnimBone * animBone, int keyCount, float tickTime, float duration) {
      std::vector<Key>& keys = animBone->keys;

      int earlyNdx = findEarlyKeyIndex(keyCount, tickTime, duration);
      int lateNdx = earlyNdx + 1;

      Key interpKey = (keyCount == 1) ? keys[earlyNdx] :
         interpolateKeys(keys[earlyNdx], keys[lateNdx], tickTime);

      return Mmath::TransformationMatrix(interpKey.position, interpKey.rotation, interpKey.scale);
   }
}
