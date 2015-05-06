
#ifndef __MATRIX_MATH_H__
#define __MATRIX_MATH_H__

#include <Eigen/Dense>
#define EIGEN_DEFAULT_TO_COLUMN_MAJOR

namespace Mmath {
   template <typename T>
   Eigen::Matrix<T,4,4> TranslationMatrix(
      const Eigen::Matrix<T,3,1> tns
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m.block(0,3,3,1) = tns;
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> RotationMatrix(
      const Eigen::Quaternion<T> rot
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m.block(0,0,3,3) = rot.toRotationMatrix();
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> ScaleMatrix(
      const Eigen::Matrix<T,3,1> scl
   ) {
      Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
      m(0,0) = scl(0);
      m(1,1) = scl(1);
      m(2,2) = scl(2);
      return m;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> TransformationMatrix(
      const Eigen::Matrix<T,3,1> tns,
      const Eigen::Quaternion<T> rot,
      const Eigen::Matrix<T,3,1> scl
   ) {
      return TranslationMatrix(tns) * RotationMatrix(rot) * ScaleMatrix(scl);
   }

   template <typename T>
   Eigen::Matrix<T,4,4> AngleAxisMatrix(T angle, Eigen::Matrix<T,3,1> axis) {
      Eigen::Quaternion<T> rotQuat(Eigen::AngleAxis<T>(angle, axis));
      return RotationMatrix(rotQuat);
   }

   template <typename T>
   Eigen::Matrix<T,3,1> RotateVec3(
      const Eigen::Matrix<T,3,1> subject,
      const T angle,
      const Eigen::Matrix<T,3,1> axis
   ) {
      Eigen::Matrix<T,3,3> rotM;
      rotM = Eigen::AngleAxis<T>(angle, axis);
      return rotM * subject;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> PerspectiveMatrix(
      const T fovy,
      const T aspect,
      const T zNear,
      const T zFar
   ) {
      T tanHalfFovy = tan(fovy / static_cast<T>(2));

      Eigen::Matrix<T,4,4> Result;
      Result.setZero();
      Result(0,0) = static_cast<T>(1) / (aspect * tanHalfFovy);
      Result(1,1) = static_cast<T>(1) / (tanHalfFovy);
      Result(2,2) = - (zFar + zNear) / (zFar - zNear);
      Result(3,2) = - static_cast<T>(1);
      Result(2,3) = - (static_cast<T>(2) * zFar * zNear) / (zFar - zNear);

      return Result;
   }

   template <typename T>
   Eigen::Matrix<T,4,4> LookAtMatrix(
      const Eigen::Matrix<T,3,1> eye,
      const Eigen::Matrix<T,3,1> direction,
      const Eigen::Matrix<T,3,1> up
   ) {
      Eigen::Matrix<T,3,1> f(direction);
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

   template <typename T>
   Eigen::Matrix<T,3,3> InverseTBN(
      const Eigen::Matrix<T,3,1> tangent,
      const Eigen::Matrix<T,3,1> bitangent,
      const Eigen::Matrix<T,3,1> normal
   ) {
      Eigen::Matrix<T,3,3> iTBN;
      iTBN.block(0,0,1,3) = tangent.transpose();
      iTBN.block(1,0,1,3) = bitangent.transpose();
      iTBN.block(2,0,1,3) = normal.transpose();
      return iTBN;
   }
}

#endif // __MATRIX_MATH_H__
