
#include "model.h"
#include "Eigen/Dense"

void Model::loadSkinningPIN(const char * path) {

}

void Model::loadAnimationPIN(const char * path) {

}


// //
// // Copyright 2012-2013, Syoyo Fujita.
// //
// // Licensed under 2-clause BSD liecense.
// //
// #ifndef _ATTACHMENT_LOADER_H
// #define _ATTACHMENT_LOADER_H

// #include <string>
// #include <vector>
// #include <map>

// // typedef struct {
// //    int numBones;


// // } skeleton;

// void AL_load(const char * attPath,
//              const char * skelPath,
//              std::vector<float> &boneWeights,      // by vertex
//              std::vector<float> &frames, // by animation frame
//              std::vector<float> &bindPose,
//              int& numBones);        // by bone

// #endif  // _ATTACHMENT_LOADER_H



// #include <cstdlib>
// #include <cstring>
// #include <cassert>

// #include <string>
// #include <vector>
// #include <map>
// #include <fstream>
// #include <sstream>
// #include <iostream>

// #include "attachment_loader.h"

// static inline bool isSpace(const char c) {
//   return (c == ' ') || (c == '\t');
// }

// static inline bool isNewLine(const char c) {
//   return (c == '\r') || (c == '\n') || (c == '\0');
// }

// static inline int parseInt(const char*& token)
// {
//   token += strspn(token, " \t");
//   int i = atoi(token);
//   token += strcspn(token, " \t\r");
//   return i;
// }

// static inline float parseFloat(const char*& token)
// {
//   token += strspn(token, " \t");
//   float f = (float)atof(token);
//   token += strcspn(token, " \t\r");
//   return f;
// }

// static inline void parseFloat3(
//   float& x, float& y, float& z,
//   const char*& token)
// {
//   x = parseFloat(token);
//   y = parseFloat(token);
//   z = parseFloat(token);
// }

// void parseNumBones(int& numBones, std::istream& inStream) {
//    int maxchars = 8192;  // Alloc enough size.
//    std::vector<char> buf(maxchars);  // Alloc enough size.

//    while (inStream.peek() != -1) {
//       inStream.getline(&buf[0], maxchars);

//       std::string linebuf(&buf[0]);

//       // Trim newline '\r\n' or '\n'
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\n')
//             linebuf.erase(linebuf.size()-1);
//       }
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\r')
//             linebuf.erase(linebuf.size()-1);
//       }

//       // Skip if empty line.
//       if (linebuf.empty()) {
//          continue;
//       }

//       // Skip leading space.
//       const char* token = linebuf.c_str();
//       token += strspn(token, " \t");

//       assert(token);
//       if (token[0] == '\0') continue; // empty line
//       if (token[0] == '#') continue;  // comment line

//       // parse the bones count
//       parseInt(token);
//       token++;
//       numBones = parseInt(token);
//       break;
//    }
// }

// static void loadWeights(std::vector<float>& boneWeights,
//                         int& numBones,
//                         std::istream& inStream) {

//    parseNumBones(numBones, inStream);

//    int maxchars = 8192;  // Alloc enough size.
//    std::vector<char> buf(maxchars);  // Alloc enough size.

//    while (inStream.peek() != -1) {
//       inStream.getline(&buf[0], maxchars);

//       std::string linebuf(&buf[0]);

//       // Trim newline '\r\n' or '\n'
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\n')
//             linebuf.erase(linebuf.size()-1);
//       }
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\r')
//             linebuf.erase(linebuf.size()-1);
//       }

//       // Skip if empty line.
//       if (linebuf.empty()) {
//          continue;
//       }

//       // Skip leading space.
//       const char* token = linebuf.c_str();
//       token += strspn(token, " \t");

//       assert(token);
//       if (token[0] == '\0') continue; // empty line
//       if (token[0] == '#') continue;  // comment line

//       // Read the floats
//       while (!isNewLine(token[0])) {
//          if (!isSpace(token[0])) {
//             float val = parseFloat(token);
//             boneWeights.push_back(val);
//          } else
//             token++;
//       }
//    }
// }

// static void parseBindPose(std::vector<float>& bindPose,
//                           std::istream& inStream) {

//    int maxchars = 8192;  // Alloc enough size.
//    std::vector<char> buf(maxchars);  // Alloc enough size.

//    inStream.getline(&buf[0], maxchars);
//    std::string linebuf(&buf[0]);

//    // Trim newline '\r\n' or '\n'
//    if (linebuf.size() > 0) {
//       if (linebuf[linebuf.size()-1] == '\n')
//          linebuf.erase(linebuf.size()-1);
//    }
//    if (linebuf.size() > 0) {
//       if (linebuf[linebuf.size()-1] == '\r')
//          linebuf.erase(linebuf.size()-1);
//    }

//    // Skip leading space.
//    const char* token = linebuf.c_str();
//    token += strspn(token, " \t");

//    while (!isNewLine(token[0])) {
//       if (!isSpace(token[0])) {
//          float val = parseFloat(token);
//          bindPose.push_back(val);
//       } else
//          token++;
//    }
// }

// static void loadSkeleton(std::vector<float>& frames,
//                          std::vector<float>& bindPose,
//                          std::istream& inStream) {

//    int numBones;
//    parseNumBones(numBones, inStream);

//    int maxchars = 8192;  // Alloc enough size.
//    std::vector<char> buf(maxchars);  // Alloc enough size.

//    parseBindPose(bindPose, inStream);

//    while (inStream.peek() != -1) {
//       inStream.getline(&buf[0], maxchars);
//       std::string linebuf(&buf[0]);

//       // Trim newline '\r\n' or '\n'
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\n')
//             linebuf.erase(linebuf.size()-1);
//       }
//       if (linebuf.size() > 0) {
//          if (linebuf[linebuf.size()-1] == '\r')
//             linebuf.erase(linebuf.size()-1);
//       }

//       // Skip if empty line.
//       if (linebuf.empty()) {
//          continue;
//       }

//       // Skip leading space.
//       const char* token = linebuf.c_str();
//       token += strspn(token, " \t");

//       assert(token);
//       if (token[0] == '\0') continue; // empty line
//       if (token[0] == '#') continue;  // comment line

//       // Read the floats
//       while (!isNewLine(token[0])) {
//          if (!isSpace(token[0])) {
//             float val = parseFloat(token);
//             frames.push_back(val);
//          } else
//             token++;
//       }
//    }
// }


// void AL_load(const char * attPath,
//              const char * skelPath,
//              std::vector<float> &boneWeights,      // by vertex
//              std::vector<float> &frames, // by animation frame
//              std::vector<float> &bindPose,
//              int& numBones) {       // by bone

//    std::stringstream err;

//    std::ifstream ifsATT(attPath);
//    if (!ifsATT) {
//       err << "Cannot open file [" << attPath << "]" << std::endl;
//       std::cerr << err.str();
//       exit(1);
//    }

//    loadWeights(boneWeights, numBones, ifsATT);

//    std::ifstream ifsSKEL(skelPath);
//    if (!ifsSKEL) {
//       err << "Cannot open file [" << attPath << "]" << std::endl;
//       std::cerr << err.str();
//       exit(1);
//    }

//    loadSkeleton(frames, bindPose, ifsSKEL);
// }



// ==================================================


// #include <iostream>
// #include "ShapeObj.h"
// #include "attachment_loader.h"
// #include "GLSL.h"

// using namespace std;

// #define MAX_INFLUENCES 15

// typedef struct {
//    unsigned int index;
//    float weight;
// } BoneWeight;

// typedef struct Vertex {
//    std::vector<BoneWeight> boneWeights;
// } Vertex;

// static bool weightCompare(BoneWeight a, BoneWeight b) {
//    return a.weight > b.weight;
// }

// void printM(Eigen::Matrix4f m) {
//    for (int i = 0; i < 4; i++) {
//       for (int j = 0; j < 4; j++)
//          printf("%0.3f ", m(i, j));
//       printf("\n");
//    }
// }

// ShapeObj::ShapeObj() :
//    posBufID(0),
//    norBufID(0),
//    texBufID(0),
//    indBufID(0)
// {
// }

// ShapeObj::~ShapeObj()
// {
// }

// void ShapeObj::load(const string &meshName, const std::string &attName, const std::string &skelName)
// {
//    // Load geometry
//    // Some obj files contain material information.
//    // We'll ignore them for this assignment.
//    vector<tinyobj::material_t> objMaterials;
//    string err = tinyobj::LoadObj(shapes, objMaterials, meshName.c_str());
//    if (!err.empty())
//       cerr << err << endl;

//    // Load the attachment and skeleton data
//    AL_load(attName.c_str(), skelName.c_str(), boneWeights, frames, bindPose, boneCount);
// }

// void ShapeObj::init(Program * prog)
// {
//    this->prog = prog;

//    bindMesh();
//    bindBoneWeights();
//    setBindPoseMatrices();
//    bindFrames();

//    prog->addUniform("animM");
//    prog->addUniform("bindM");
//    prog->addAttribute("vertPos");
//    prog->addAttribute("vertNor");
//    prog->addAttribute("bI0");
//    prog->addAttribute("bI1");
//    prog->addAttribute("bI2");
//    prog->addAttribute("bI3");
//    prog->addAttribute("bW0");
//    prog->addAttribute("bW1");
//    prog->addAttribute("bW2");
//    prog->addAttribute("bW3");
//    prog->addAttribute("influences");
// }



// void ShapeObj::bindBoneWeights()
// {
//    int numVertices = boneWeights.size() / 18;

//    // printf("numVertices %d\n", numVertices);
//    Vertex * verts = new Vertex[numVertices];
//    arrangeBoneWeights(verts, numVertices);

//    float bIndices[numVertices * MAX_INFLUENCES];
//    float bWeights[numVertices * MAX_INFLUENCES];
//    float bNumInfluences[numVertices];

//    for (int i = 0; i < numVertices; i++) {
//       bool foundNum = false;
//       // printf("-------------\n");
//       for (int j = 0; j < MAX_INFLUENCES; j++) {
//          // printf("size of verts [%d] boneWeights %d\n", i, verts[i].boneWeights.size());
//          bIndices[MAX_INFLUENCES * i + j] = verts[i].boneWeights[j].index;
//          bWeights[MAX_INFLUENCES * i + j] = verts[i].boneWeights[j].weight;
//          // printf("index %d weight %f\n", verts[i].boneWeights[j].index, verts[i].boneWeights[j].weight);
//          if (verts[i].boneWeights[j].weight == 0.0f && !foundNum) {
//             // printf("%d %d\n", i, j);
//             bNumInfluences[i] = j;
//             foundNum = true;
//          }
//       }

//       // std::cerr << boneNumInfluences[i] << std::endl;
//    }

//    // for (int i = 0; i < numVertices; i++) {
//    //    for (int j = 0; j < MAX_INFLUENCES)
//    //    if (int(bNumInfluences[i] > 15))
//    //       printf("numInfluences = %d\n", int(bNumInfluences[i]));
//    // }

//    // std::cerr << bWeights.size
//    int bufLen = sizeof(float) * numVertices * MAX_INFLUENCES;

//    glGenBuffers(1, & bIndID);
//    glBindBuffer(GL_ARRAY_BUFFER, bIndID);
//    glBufferData(GL_ARRAY_BUFFER, bufLen, & bIndices[0], GL_STATIC_DRAW);

//    glGenBuffers(1, & bWeightID);
//    glBindBuffer(GL_ARRAY_BUFFER, bWeightID);
//    glBufferData(GL_ARRAY_BUFFER, bufLen, & bWeights[0], GL_STATIC_DRAW);

//    glGenBuffers(1, & bNumWeightsID);
//    glBindBuffer(GL_ARRAY_BUFFER, bNumWeightsID);
//    glBufferData(GL_ARRAY_BUFFER, bufLen, & bNumInfluences[0], GL_STATIC_DRAW);
// }



// void ShapeObj::arrangeBoneWeights(Vertex * verts, int vertCount)
// {
//    // for (int i = 0; i < boneWeights.size(); i++) {
//    //    printf("%f\n", boneWeights[i]);
//    // }

//    // put each bone weight into each vertex's boneWeights list
//    // printf("boneWeights size %d\n", boneWeights.size());
//    for (int i = 0; i < boneWeights.size(); i++) {
//       int boneNum = i % boneCount;
//       int vertNdx = i / boneCount;
//       BoneWeight bw;
//       bw.index = boneNum;
//       bw.weight = boneWeights[i];

//       verts[vertNdx].boneWeights.push_back(bw);
//    }

//    // for (int i = 0; i < 10; i++) {
//    //    for (int j = 0; j < verts[i].boneWeights.size(); j++)
//    //       printf("%d-%0.3f, ", verts[i].boneWeights[j].index, verts[i].boneWeights[j].weight);
//    //    printf("\n");
//    // }

//    // sort the bone indices by their bone weights (in decending order)
//    for (int i = 0; i < vertCount; i++)
//       std::sort(verts[i].boneWeights.begin(), verts[i].boneWeights.end(), weightCompare);
// }

// void ShapeObj::setBindPoseMatrices()
// {
//    for (int i = 0; i < boneCount; i++) {
//       Eigen::Quaternionf q;
//       q.vec() << bindPose[7*i], bindPose[7*i+1], bindPose[7*i+2];
//       q.w() = bindPose[7*i+3];

//       Eigen::Vector3f p;
//       p << bindPose[7*i+4], bindPose[7*i+5], bindPose[7*i+6];

//       Eigen::Matrix4f E = Eigen::Matrix4f::Identity();
//       E.block<3,3>(0,0) = q.toRotationMatrix();
//       E.block<3,1>(0,3) = p;

//       // bindM[i] = Eigen::Matrix4f::Identity();
//       bindM[i] = E.inverse();
//    }
// }

// void ShapeObj::bindFrames()
// {
//    frameCount = frames.size() / (7 * boneCount);

//    for (int i = 0; i < boneCount; i++) {
//       for (int j = 0; j < frameCount; j++) {
//          int sInd = frameCount*i + 7*j;

//          Eigen::Quaternionf q;
//          q.vec() << frames[sInd], frames[sInd+1], frames[sInd+2];
//          q.w() = bindPose[sInd+3];

//          Eigen::Vector3f p;
//          p << bindPose[sInd+4], bindPose[sInd+5], bindPose[sInd+6];

//          Eigen::Matrix4f E = Eigen::Matrix4f::Identity();
//          E.block<3,3>(0,0) = q.toRotationMatrix();
//          E.block<3,1>(0,3) = p;

//          boneFrames[i].push_back(E);
//       }
//    }

//    // printM(boneFrames[0][0]);
// }

// void ShapeObj::draw(float t) const
// {
//    int h_pos = prog->getAttribute("vertPos");
//    int h_nor = prog->getAttribute("vertNor");
//    int h_tex = -1;
//    int h_bI0 = prog->getAttribute("bI0");
//    int h_bI1 = prog->getAttribute("bI1");
//    int h_bI2 = prog->getAttribute("bI2");
//    int h_bI3 = prog->getAttribute("bI3");
//    int h_bW0 = prog->getAttribute("bW0");
//    int h_bW1 = prog->getAttribute("bW1");
//    int h_bW2 = prog->getAttribute("bW2");
//    int h_bW3 = prog->getAttribute("bW3");
//    int h_influences = prog->getAttribute("influences");
//    int h_bindM = prog->getUniform("bindM");
//    int h_animM = prog->getUniform("animM");

//    // uniform bind and anim matrices for each bone
//    // printf("%f\n", t);
//    int frameNdx = int(t) % frameCount;

//    vector<Eigen::Matrix4f> animM = vector<Eigen::Matrix4f>(18);
//    for (int i = 0; i < 18; i++)
//       animM[i] = boneFrames[i][frameNdx];

//    glUniformMatrix4fv(h_animM, 18, GL_FALSE, animM[0].data());
//    glUniformMatrix4fv(h_bindM, 18, GL_FALSE, bindM[0].data());

//    // Enable and bind position array for drawing
//    GLSL::enableVertexAttribArray(h_pos);
//    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
//    glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

//    // Enable and bind normal array (if it exists) for drawing
//    if(norBufID && h_nor >= 0) {
//       GLSL::enableVertexAttribArray(h_nor);
//       glBindBuffer(GL_ARRAY_BUFFER, norBufID);
//       glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    }

//    // Enable and bind texcoord array (if it exists) for drawing
//    if(texBufID && h_tex >= 0) {
//       GLSL::enableVertexAttribArray(h_tex);
//       glBindBuffer(GL_ARRAY_BUFFER, texBufID);
//       glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
//    }

//    // Bone Indices
//    GLSL::enableVertexAttribArray(h_bI0);
//    GLSL::enableVertexAttribArray(h_bI1);
//    GLSL::enableVertexAttribArray(h_bI2);
//    GLSL::enableVertexAttribArray(h_bI3);
//    glBindBuffer(GL_ARRAY_BUFFER, bIndID);
//    glVertexAttribPointer(h_bI0, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 0*sizeof(float)));
//    glVertexAttribPointer(h_bI1, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 4*sizeof(float)));
//    glVertexAttribPointer(h_bI2, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 8*sizeof(float)));
//    glVertexAttribPointer(h_bI3, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)(12*sizeof(float)));

//    // Bone Weights
//    GLSL::enableVertexAttribArray(h_bW0);
//    GLSL::enableVertexAttribArray(h_bW1);
//    GLSL::enableVertexAttribArray(h_bW2);
//    GLSL::enableVertexAttribArray(h_bW3);
//    glBindBuffer(GL_ARRAY_BUFFER, bWeightID);
//    glVertexAttribPointer(h_bW0, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 0*sizeof(float)));
//    glVertexAttribPointer(h_bW1, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 4*sizeof(float)));
//    glVertexAttribPointer(h_bW2, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)( 8*sizeof(float)));
//    glVertexAttribPointer(h_bW3, 4, GL_FLOAT, GL_FALSE, 16*sizeof(float), (const void *)(12*sizeof(float)));

//    // Bone number of influences
//    GLSL::enableVertexAttribArray(h_influences);
//    glBindBuffer(GL_ARRAY_BUFFER, bNumWeightsID);
//    glVertexAttribPointer(h_influences, 1, GL_FLOAT, GL_FALSE, 0, 0);

//    // Bind index array for drawing
//    int nIndices = (int)shapes[0].mesh.indices.size();
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);

//    // Draw
//    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

//    // Disable and unbind
//    if(texBufID && h_tex >= 0) {
//       GLSL::disableVertexAttribArray(h_tex);
//    }
//    if(norBufID && h_nor >= 0) {
//       GLSL::disableVertexAttribArray(h_nor);
//    }
//    GLSL::disableVertexAttribArray(h_pos);

//    GLSL::disableVertexAttribArray(h_bI0);
//    GLSL::disableVertexAttribArray(h_bI1);
//    GLSL::disableVertexAttribArray(h_bI2);
//    GLSL::disableVertexAttribArray(h_bI3);

//    GLSL::disableVertexAttribArray(h_bW0);
//    GLSL::disableVertexAttribArray(h_bW1);
//    GLSL::disableVertexAttribArray(h_bW2);
//    GLSL::disableVertexAttribArray(h_bW3);

//    GLSL::disableVertexAttribArray(h_influences);

//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
// }