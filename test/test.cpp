#include <Eigen/Dense>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "grid.h"
#include "model.h"

void printDeque(std::deque<int>& d) {
   printf("size = %d\n", d.size());
   for (int i = 0; i < d.size(); i++)
      printf("%d: %d\n", i, d[i]);
}

// class

int main() {
   SpatialGrid< grid = SpatialGrid(10, 1);
   grid.Add()


   return 0;
}