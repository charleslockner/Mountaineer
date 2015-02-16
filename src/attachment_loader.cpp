
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "attachment_loader.h"

static inline bool isSpace(const char c) {
  return (c == ' ') || (c == '\t');
}

static inline bool isNewLine(const char c) {
  return (c == '\r') || (c == '\n') || (c == '\0');
}

static inline int parseInt(const char*& token)
{
  token += strspn(token, " \t");
  int i = atoi(token);
  token += strcspn(token, " \t\r");
  return i;
}

static inline float parseFloat(const char*& token)
{
  token += strspn(token, " \t");
  float f = (float)atof(token);
  token += strcspn(token, " \t\r");
  return f;
}

static inline void parseFloat3(
  float& x, float& y, float& z,
  const char*& token)
{
  x = parseFloat(token);
  y = parseFloat(token);
  z = parseFloat(token);
}

static void parseNumBones(int& numBones, std::istream& inStream) {
   int maxchars = 8192;  // Alloc enough size.
   std::vector<char> buf(maxchars);  // Alloc enough size.

   while (inStream.peek() != -1) {
      inStream.getline(&buf[0], maxchars);

      std::string linebuf(&buf[0]);

      // Trim newline '\r\n' or '\n'
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\n')
            linebuf.erase(linebuf.size()-1);
      }
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\r')
            linebuf.erase(linebuf.size()-1);
      }

      // Skip if empty line.
      if (linebuf.empty()) {
         continue;
      }

      // Skip leading space.
      const char* token = linebuf.c_str();
      token += strspn(token, " \t");

      assert(token);
      if (token[0] == '\0') continue; // empty line
      if (token[0] == '#') continue;  // comment line

      // parse the bones count
      parseInt(token);
      token++;
      numBones = parseInt(token);
      break;
   }
}

void PIN_loadWeights(std::vector<float>& boneWeights,
                     int& numBones,
                     const char * path) {

   std::stringstream err;
   std::ifstream ifsATT(path);
   if (!ifsATT) {
      err << "Cannot open file [" << path << "]" << std::endl;
      std::cerr << err.str();
      exit(1);
   }

   std::istream& inStream = ifsATT;

   parseNumBones(numBones, inStream);

   int maxchars = 8192;  // Alloc enough size.
   std::vector<char> buf(maxchars);  // Alloc enough size.

   while (inStream.peek() != -1) {
      inStream.getline(&buf[0], maxchars);

      std::string linebuf(&buf[0]);

      // Trim newline '\r\n' or '\n'
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\n')
            linebuf.erase(linebuf.size()-1);
      }
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\r')
            linebuf.erase(linebuf.size()-1);
      }

      // Skip if empty line.
      if (linebuf.empty()) {
         continue;
      }

      // Skip leading space.
      const char* token = linebuf.c_str();
      token += strspn(token, " \t");

      assert(token);
      if (token[0] == '\0') continue; // empty line
      if (token[0] == '#') continue;  // comment line

      // Read the floats
      while (!isNewLine(token[0])) {
         if (!isSpace(token[0])) {
            float val = parseFloat(token);
            boneWeights.push_back(val);
         } else
            token++;
      }
   }
}

static void parseBindPose(std::vector<float>& bindPose,
                          std::istream& inStream) {

   int maxchars = 8192;  // Alloc enough size.
   std::vector<char> buf(maxchars);  // Alloc enough size.

   inStream.getline(&buf[0], maxchars);
   std::string linebuf(&buf[0]);

   // Trim newline '\r\n' or '\n'
   if (linebuf.size() > 0) {
      if (linebuf[linebuf.size()-1] == '\n')
         linebuf.erase(linebuf.size()-1);
   }
   if (linebuf.size() > 0) {
      if (linebuf[linebuf.size()-1] == '\r')
         linebuf.erase(linebuf.size()-1);
   }

   // Skip leading space.
   const char* token = linebuf.c_str();
   token += strspn(token, " \t");

   while (!isNewLine(token[0])) {
      if (!isSpace(token[0])) {
         float val = parseFloat(token);
         bindPose.push_back(val);
      } else
         token++;
   }
}

void PIN_loadSkeleton(std::vector<float>& frames,
                      std::vector<float>& bindPose,
                      int& numBones,
                      const char * path) {

   std::stringstream err;
   std::ifstream ifsSKEL(path);
   if (!ifsSKEL) {
      err << "Cannot open file [" << path << "]" << std::endl;
      std::cerr << err.str();
      exit(1);
   }

   std::istream& inStream = ifsSKEL;

   parseNumBones(numBones, inStream);

   int maxchars = 8192;  // Alloc enough size.
   std::vector<char> buf(maxchars);  // Alloc enough size.

   parseBindPose(bindPose, inStream);

   while (inStream.peek() != -1) {
      inStream.getline(&buf[0], maxchars);
      std::string linebuf(&buf[0]);

      // Trim newline '\r\n' or '\n'
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\n')
            linebuf.erase(linebuf.size()-1);
      }
      if (linebuf.size() > 0) {
         if (linebuf[linebuf.size()-1] == '\r')
            linebuf.erase(linebuf.size()-1);
      }

      // Skip if empty line.
      if (linebuf.empty()) {
         continue;
      }

      // Skip leading space.
      const char* token = linebuf.c_str();
      token += strspn(token, " \t");

      assert(token);
      if (token[0] == '\0') continue; // empty line
      if (token[0] == '#') continue;  // comment line

      // Read the floats
      while (!isNewLine(token[0])) {
         if (!isSpace(token[0])) {
            float val = parseFloat(token);
            frames.push_back(val);
         } else
            token++;
      }
   }
}
