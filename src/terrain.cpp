
#include "terrain.h"
#include "stdio.h"

using namespace Eigen;

TerrainGenerator::TerrainGenerator() {}
TerrainGenerator::~TerrainGenerator() {}

static double randRange(float low, float high) {
    return (high - low) * rand() / (float)RAND_MAX - low;
}

static int rcToIndex(int width, int row, int col) {
   return width * row + col;
}

// Returns the normalized cross product with the up vector (v x up)
// Returns vec3(0,0,1) if v == up
static Vector3f crossWithUp(Vector3f v) {
   Vector3f r = v.cross(Vector3f(0,1,0));
   return (r(0) || r(1) || r(2)) ? r.normalized() : Vector3f(0,0,1);
}

static float square(float f) {
   return f * f;
}

template <typename T>
static int pntrToNdx(std::vector<T>& cont, T * elem) {
   return elem - &cont[0];
}

Model * TerrainGenerator::GenerateModel() {
   edgeLength = 1;
   float halfLength = edgeLength / 2.0f;

   // The general direction the mesh is heading towards
   Vector3f ultimateDirection = Vector3f(0.25,1.0,-0.25).normalized();

   Vector3f bitangent = ultimateDirection;
   Vector3f tangent = crossWithUp(bitangent);
   Vector3f normal = tangent.cross(bitangent).normalized();

   // Set up the beginning triangle
   Vertex vTop;
   vTop.normal = normal;
   vTop.tangent = tangent;
   vTop.bitangent = bitangent;
   vTop.position = halfLength * bitangent;
   vTop.uv = Vector2f(0.5f, 1.0f);

   Vertex vLeft;
   vLeft.normal = normal;
   vLeft.tangent = tangent;
   vLeft.bitangent = bitangent;
   vLeft.position = -halfLength * bitangent - halfLength * tangent;
   vLeft.uv = Vector2f(0.0f, 0.0f);

   Vertex vRight;
   vRight.normal = normal;
   vRight.tangent = tangent;
   vRight.bitangent = bitangent;
   vRight.position = -halfLength * bitangent + halfLength * tangent;
   vRight.uv = Vector2f(1.0f, 0.0f);

   Face face;
   face.vertIndices[0] = 0;
   face.vertIndices[1] = 1;
   face.vertIndices[2] = 2;
   face.normal = normal;

   // Initialize the model
   model = new Model();
   model->vertices = std::vector<Vertex>(0);
   model->vertices.reserve(1000000);
   model->faces = std::vector<Face>(0);
   model->faces.reserve(1000000);
   model->vertices.push_back(vTop);
   model->vertices.push_back(vLeft);
   model->vertices.push_back(vRight);
   model->faces.push_back(face);
   model->hasNormals = true;
   model->hasTexCoords = true;
   model->hasTansAndBitans = true;
   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   // Send the triangle data to the shader
   model->bufferVertices();
   model->bufferIndices();

   // Create the first 6 paths
   paths = std::vector<Path *>(0);
   Path * p;

   p = new Path();
   p->headV = & model->vertices[0];
   p->tailV = & model->vertices[1];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[0];
   p->tailV = & model->vertices[2];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[1];
   p->tailV = & model->vertices[2];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[1];
   p->tailV = & model->vertices[0];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[2];
   p->tailV = & model->vertices[0];
   paths.push_back(p);
   p = new Path();
   p->headV = & model->vertices[2];
   p->tailV = & model->vertices[1];
   paths.push_back(p);

   paths[0]->leftP = paths[1];
   paths[0]->rightP = paths[5];
   paths[1]->leftP = paths[2];
   paths[1]->rightP = paths[0];
   paths[2]->leftP = paths[3];
   paths[2]->rightP = paths[1];
   paths[3]->leftP = paths[4];
   paths[3]->rightP = paths[2];
   paths[4]->leftP = paths[5];
   paths[4]->rightP = paths[3];
   paths[5]->leftP = paths[0];
   paths[5]->rightP = paths[4];

   stepCnt = 0;

   return model;
}

void TerrainGenerator::BuildStep() {
   int numPaths = paths.size();

   // Extend each path to build the vertices
   for (int i = 0; i < numPaths; i++) {
      Path * p = paths[i];
      Vector3f heading = (p->headV->position - p->tailV->position).normalized();

      // Add vertex created from extending the path
      Vertex v;
      v.position = p->headV->position + edgeLength * heading;
      v.tangent = p->headV->tangent;
      v.bitangent = p->headV->bitangent;
      v.normal = p->headV->normal;
      Matrix3f iTBN = Mmath::InverseTBN(v.tangent, v.bitangent, v.normal);
      v.uv = p->headV->uv + (iTBN * heading).head<2>();
      model->vertices.push_back(v);

      // Update the path head and tail vertices
      p->tailV = p->headV;
      p->headV = & model->vertices.back();
   }

   // Go through each path to create new paths and faces
   for (int i = 0; i < numPaths; i++) {
      Path * selfPath = paths[i];
      Path * rightPath = selfPath->rightP;

      // If the paths have the same tail
      if (selfPath->tailV == rightPath->tailV) {
         // Create the emerging face
         Face f;
         f.vertIndices[0] = pntrToNdx(model->vertices, selfPath->headV);
         f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
         f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->headV);
         model->faces.push_back(f);

      } else { // If the paths have different tails
         Vector3f selfHeadPos = selfPath->headV->position;
         Vector3f rightHeadPos = rightPath->headV->position;

         // If the distance between the two heads is ok
         if ((selfHeadPos - rightHeadPos).squaredNorm() < square(1.5 * edgeLength)) {
            Vector3f selfTailPos = selfPath->tailV->position;
            Vector3f rightTailPos = rightPath->tailV->position;
            Face f;

            // Create 2 faces to fill the space
            if ((rightHeadPos - selfTailPos).squaredNorm() < (selfHeadPos - rightTailPos).squaredNorm()) {
               f.vertIndices[0] = pntrToNdx(model->vertices, selfPath->headV);
               f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
               f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->headV);
               model->faces.push_back(f);

               f.vertIndices[0] = pntrToNdx(model->vertices, rightPath->headV);
               f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
               f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->tailV);
               model->faces.push_back(f);
            } else {
               f.vertIndices[0] = pntrToNdx(model->vertices, selfPath->headV);
               f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
               f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->tailV);
               model->faces.push_back(f);

               f.vertIndices[0] = pntrToNdx(model->vertices, rightPath->headV);
               f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->headV);
               f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->tailV);
               model->faces.push_back(f);
            }

         } else { // If the distance between the two heads is too big
            // Add a vertex between them
            Vertex v;
            v.position = 0.5f * (selfHeadPos + rightHeadPos);
            v.tangent = selfPath->tailV->tangent;
            v.bitangent = selfPath->tailV->bitangent;
            v.normal = selfPath->tailV->normal;
            Matrix3f iTBN = Mmath::InverseTBN(v.tangent, v.bitangent, v.normal);
            v.uv = selfPath->tailV->uv + (iTBN * (v.position - selfPath->tailV->position)).head<2>();

            model->vertices.push_back(v);
            Vertex * midV = & model->vertices.back();

            // Create 2 new paths who's head is the new vertex
            Path * midRightPath = new Path();
            Path * midSelfPath = new Path();

            midRightPath->headV = midV;
            midRightPath->tailV = selfPath->tailV;
            midRightPath->leftP = midSelfPath;
            midRightPath->rightP = rightPath;

            midSelfPath->headV = midV;
            midSelfPath->tailV = rightPath->tailV;
            midSelfPath->leftP = selfPath;
            midSelfPath->rightP = midRightPath;

            paths.push_back(midRightPath);
            paths.push_back(midSelfPath);

            // Update the paths to the left and right of the created ones
            selfPath->rightP = midSelfPath;
            rightPath->leftP = midRightPath;

            // Create the emerging triangle faces
            Face f;
            f.vertIndices[0] = pntrToNdx(model->vertices, selfPath->headV);
            f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
            f.vertIndices[2] = pntrToNdx(model->vertices, midV);
            model->faces.push_back(f);

            f.vertIndices[0] = pntrToNdx(model->vertices, midV);
            f.vertIndices[1] = pntrToNdx(model->vertices, rightPath->tailV);
            f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->headV);
            model->faces.push_back(f);

            f.vertIndices[0] = pntrToNdx(model->vertices, midV);
            f.vertIndices[1] = pntrToNdx(model->vertices, selfPath->tailV);
            f.vertIndices[2] = pntrToNdx(model->vertices, rightPath->tailV);
            model->faces.push_back(f);
         }
      }
   }

   model->vertexCount = model->vertices.size();
   model->faceCount = model->faces.size();
   model->bufferVertices();
   model->bufferIndices();

   stepCnt++;
   printf("Step [%d]: paths %d, verts %d faces %d\n", stepCnt, paths.size(), model->vertices.size(), model->faces.size());
}





