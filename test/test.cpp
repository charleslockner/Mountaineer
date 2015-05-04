#include <Eigen/Dense>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>

void printDeque(std::deque<int>& d) {
   printf("size = %d\n", d.size());
   for (int i = 0; i < d.size(); i++)
      printf("%d: %d\n", i, d[i]);
}

int main() {
   std::deque<int> d = std::deque<int>(10);

   printf("rounding: %d\n", (int)(-1.5/2));

   printDeque(d);

   d.push_back(1);
   d.push_back(2);
   d.push_back(3);
   d.push_back(4);

   printDeque(d);

   d.pop_front();

   printDeque(d);

   d.push_front(11);
   d.push_front(12);
   d.push_front(13);

   printDeque(d);

   d.pop_front();

   printDeque(d);

   d[100] = 100;

   printDeque(d);


   return 0;
}