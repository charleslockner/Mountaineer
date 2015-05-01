#include <Eigen/Dense>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
   Eigen::Vector3f a;
   Eigen::Vector3f b;
   float c;
   Eigen::Vector3f d;
   char e;
} St;

int main() {
   St sts[3];
   sts[0].a = Eigen::Vector3f(1,2,3);
   sts[0].b = Eigen::Vector3f(3,4,5);
   sts[0].c = 6;
   sts[0].d = Eigen::Vector3f(7,8,9);

   sts[1].a = Eigen::Vector3f(1,2,3);
   sts[1].b = Eigen::Vector3f(3,4,5);
   sts[1].c = 6;
   sts[1].d = Eigen::Vector3f(7,8,9);

   sts[2].a = Eigen::Vector3f(1,2,3);
   sts[2].b = Eigen::Vector3f(3,4,5);
   sts[2].c = 6;
   sts[2].d = Eigen::Vector3f(7,8,9);

   // char * p = (float *)(&sts);
   // for (int i = 0; i < 3; i++) {
   //    printf("%f %f %f\n", p[sizeof(St)*i], );
   //    p++;
   // }

   return 0;
}