
#ifndef __MATRIX_MATH_H__
#define __MATRIX_MATH_H__

#include <Eigen/Dense>

template <typename T>
Eigen::Matrix<T,4,4> makeTranslationMatrix(Eigen::Matrix<T,3,1> tns) {
   Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
   m.block(0,3,3,1) = tns;
   return m;
}

template <typename T>
Eigen::Matrix<T,4,4> makeRotationMatrix(Eigen::Quaternion<T> rot) {
   Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
   m.block(0,0,3,3) = rot.toRotationMatrix();
   return m;
}

template <typename T>
Eigen::Matrix<T,4,4> makeScaleMatrix(Eigen::Matrix<T,3,1> scl) {
   Eigen::Matrix<T,4,4> m = Eigen::Matrix<T,4,4>::Identity();
   m(0,0) = scl(0,0);
   m(1,1) = scl(1,0);
   m(2,2) = scl(2,0);
   return m;
}

template <typename T>
Eigen::Matrix<T,4,4> makeTransformationMatrix(Eigen::Matrix<T,3,1> tns, Eigen::Quaternion<T> rot, Eigen::Matrix<T,3,1> scl) {
   return makeTranslationMatrix(tns) * makeRotationMatrix(rot) * makeScaleMatrix(scl);
}

template <typename T>
Eigen::Matrix<T,4,4> makePerspectiveMatrix(T fovy, T aspect, T zNear, T zFar) {
   T tanHalfFovy = tan(fovy / static_cast<T>(2));

   Eigen::Matrix<T,4,4> Result; //= Eigen::Matrix<T,4,4>::Identity();
   Result << T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0), T(0);
   Result(0,0) = static_cast<T>(1) / (aspect * tanHalfFovy);
   Result(1,1) = static_cast<T>(1) / (tanHalfFovy);
   Result(2,2) = - (zFar + zNear) / (zFar - zNear);
   Result(2,3) = - static_cast<T>(1);
   Result(3,2) = - (static_cast<T>(2) * zFar * zNear) / (zFar - zNear);

   return Result;
}

template <typename T>
Eigen::Matrix<T,4,4> makeLookAtMatrix(const Eigen::Matrix<T,3,1> eye, const Eigen::Matrix<T,3,1> target, const Eigen::Matrix<T,3,1> up) {
   Eigen::Matrix<T,3,1> f((target - eye).normalized());
   Eigen::Matrix<T,3,1> s((f.cross(up)).normalized());
   Eigen::Matrix<T,3,1> u(s.cross(f));

   Eigen::Matrix<T,4,4> Result = Eigen::Matrix<T,4,4>::Identity();
   Result(0,0) = s(0);
   Result(1,0) = s(1);
   Result(2,0) = s(2);
   Result(0,1) = u(0);
   Result(1,1) = u(1);
   Result(2,1) = u(2);
   Result(0,2) =-f(0);
   Result(1,2) =-f(1);
   Result(2,2) =-f(2);
   Result(3,0) =-s.dot(eye);
   Result(3,1) =-u.dot(eye);
   Result(3,2) = f.dot(eye);

   return Result;
}

template <typename T>
Eigen::Matrix<T,3,1> rotateVec3(const Eigen::Matrix<T,3,1> subject, const T angle, const Eigen::Matrix<T,3,1> axis) {
   Eigen::Matrix<T,3,3> rotM;
   rotM = Eigen::AngleAxis<T>(angle, axis);
   return rotM * subject;
}

#endif // __MATRIX_MATH_H__
