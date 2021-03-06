#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>

#include "model.h"

void Model::loadConstraints(const char * path) {
   for (int i = 0; i < this->bones.size(); i++)
      this->bones[i].joints.clear();

   std::ifstream infile(path);
   if (!infile) {
     fprintf(stderr, "Joint file \"%s\" not found.\n", path);
     exit(1);
   }

   std::string line;
   int lineNum = 0;

   while (std::getline(infile, line)) {
      lineNum++;
      std::istringstream iss(line);

      int boneNum;
      float x, y, z, min, max;
      if (iss >> boneNum >> x >> y >> z >> min >> max) {
         IKJoint joint;
         joint.axis = Eigen::Vector3f(x, y, z);
         joint.minAngle = min;
         joint.maxAngle = max;
         this->bones[boneNum].joints.push_back(joint);
      } else {
         // skip the empty line
      }
   }

   fprintf(stderr, "Loaded JNT: %s\n", path);
}
