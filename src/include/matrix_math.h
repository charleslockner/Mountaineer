
#ifndef __MATRIX_MATH_H__
#define __MATRIX_MATH_H__

#include <Eigen/Dense>
#define EIGEN_DEFAULT_TO_COLUMN_MAJOR

namespace Mmath {
   template <typename T>
   Eigen::Matrix<T,4,4> translationMatrix(
      const Eigen::Matrix<T,3,1> tns
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m.block(0,3,3,1) = tns;
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> rotationMatrix(
      const Eigen::Quaternion<T> rot
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m.block(0,0,3,3) = rot.toRotationMatrix();
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> scaleMatrix(
      const Eigen::Matrix<T,3,1> scl
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m(0,0) = scl(0);
      m(1,1) = scl(1);
      m(2,2) = scl(2);
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> transformationMatrix(
      const Eigen::Matrix<T,3,1> tns,
      const Eigen::Quaternion<T> rot,
      const Eigen::Matrix<T,3,1> scl
   ) {
      return translationMatrix(tns) * rotationMatrix(rot) * scaleMatrix(scl);
   }

   template <typename T>
   Eigen::Matrix<T,3,1> rotateVec3(
      const Eigen::Matrix<T,3,1> subject,
      const T angle,
      const Eigen::Matrix<T,3,1> axis
   ) {
      Eigen::Matrix<T,3,3> rotM;
      rotM = Eigen::AngleAxis<T>(angle, axis);
      return rotM * subject;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> perspectiveMatrix(
      const T fovy,
      const T aspect,
      const T zNear,
      const T zFar
   ) {
      T tanHalfFovy = tan(fovy / static_cast<T>(2));

      Eigen::Matrix<T,4,4> Result;
      Result << T(0), T(0), T(0), T(0),
                T(0), T(0), T(0), T(0),
                T(0), T(0), T(0), T(0),
                T(0), T(0), T(0), T(0);
      Result(0,0) = static_cast<T>(1) / (aspect * tanHalfFovy);
      Result(1,1) = static_cast<T>(1) / (tanHalfFovy);
      Result(2,2) = - (zFar + zNear) / (zFar - zNear);
      Result(3,2) = - static_cast<T>(1);
      Result(2,3) = - (static_cast<T>(2) * zFar * zNear) / (zFar - zNear);

      return Result;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> lookAtMatrix(
      const Eigen::Matrix<T,3,1> eye,
      const Eigen::Matrix<T,3,1> target,
      const Eigen::Matrix<T,3,1> up
   ) {
      Eigen::Matrix<T,3,1> f((target - eye).normalized());
      Eigen::Matrix<T,3,1> s((f.cross(up)).normalized());
      Eigen::Matrix<T,3,1> u(s.cross(f));

      Eigen::Matrix<T,4,4> Result = Eigen::Matrix<T,4,4>::Identity();
      Result(0,0) = s(0);
      Result(0,1) = s(1);
      Result(0,2) = s(2);
      Result(1,0) = u(0);
      Result(1,1) = u(1);
      Result(1,2) = u(2);
      Result(2,0) =-f(0);
      Result(2,1) =-f(1);
      Result(2,2) =-f(2);
      Result(0,3) =-s.dot(eye);
      Result(1,3) =-u.dot(eye);
      Result(2,3) = f.dot(eye);

      return Result;
   }
}

#endif // __MATRIX_MATH_H__
