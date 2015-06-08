#include "reducer.h"
#include "model.h"
#include <assert.h>

// ========================================================== //
// ==================== STATIC FUNCTIONS ==================== //
// ========================================================== //

static void makeNeighbors(Vertex * a, Vertex * b) {
   a->neighbors.push_back(b);
   b->neighbors.push_back(a);
}

static void addFaceToVertexReferences(Face * f) {
   f->vertices[0]->faces.push_back(f);
   f->vertices[1]->faces.push_back(f);
   f->vertices[2]->faces.push_back(f);
}

static void removeNeighborReferences(Vertex * v) {
   int numNeighs = v->neighbors.size();
   for (int i = 0; i < numNeighs; i++) {
      Vertex * n = v->neighbors[i];
      n->neighbors.erase(find(n->neighbors.begin(), n->neighbors.end(), v));
   }
}

static void removeFaceFromVertexReferences(Face * f) {
   std::vector<Face *>::iterator it = find(f->vertices[0]->faces.begin(), f->vertices[0]->faces.end(), f);
   if (it == f->vertices[0]->faces.end())
      printf("face not found at vertex 0!\n");
   else
      f->vertices[0]->faces.erase(it);

   it = find(f->vertices[1]->faces.begin(), f->vertices[1]->faces.end(), f);
   if (it == f->vertices[1]->faces.end())
      printf("face not found at vertex 1!\n");
   else
      f->vertices[1]->faces.erase(it);

   it = find(f->vertices[2]->faces.begin(), f->vertices[2]->faces.end(), f);
   if (it == f->vertices[2]->faces.end())
      printf("face not found at vertex 2!\n");
   else
      f->vertices[2]->faces.erase(it);
}

static bool areNeighbors(Vertex * a, Vertex * b) {
   int aNumNeighs = a->neighbors.size();
   int bNumNeighs = b->neighbors.size();

   if (aNumNeighs < bNumNeighs)
      for (int i = 0; i < aNumNeighs; i++)
         if (a->neighbors[i] == b)
            return true;
   else
      for (int i = 0; i < bNumNeighs; i++)
         if (b->neighbors[i] == a)
            return true;

   return false;
}

// ========================================================== //
// ===================== MAIN FUNCTIONS ===================== //
// ========================================================== //

namespace MR {

   void Collapse(Model * model, Vertex * fromV, Vertex * toV) {
      assert(areNeighbors(fromV, toV));

      // Remove shared faces
      for (int i = 0; i < fromV->faces.size(); i++) {
         for (int j = 0; j < toV->faces.size(); j++) {
            if (fromV->faces[i] == toV->faces[j]) {
               Face * f = fromV->faces[i];
               removeFaceFromVertexReferences(f);

               model->faces.erase(find(model->faces.begin(), model->faces.end(), f));
               fromV->faces.erase(fromV->faces.begin() + i);
               toV->faces.erase(toV->faces.begin() + j);
               delete(f);

               i--;
               j--;
            }
         }
      }

      // Update from's faces' vertex references
      for (int i = 0; i < fromV->faces.size(); i++) {
         for (int j = 0; j < NUM_FACE_EDGES; j++) {
            if (fromV->faces[i]->vertices[j] == fromV) {
               fromV->faces[i]->vertices[j] = toV;
               break;
            }
         }
      }

      // Add each face of fromV to toV's face references
      for (int i = 0; i < fromV->faces.size(); i++)
         toV->faces.push_back(fromV->faces[i]);

      // Update neighbor references
      for (int i = 0; i < from->neighbors.size(); i++) {
         Vertex * fromNeighV = from->neighbors[i];
         // Make fromNeighV and toV neighbors if not already
         if (!areNeighbors(fromNeighV, toV) && fromNeighV != toV)
            makeNeighbors(fromNeighV, toV)
      }

      // Remove all references to fromV from fromV's neighbors
      removeNeighborReferences(fromV);

      // Remove fromV from model's vertex list
      int ndx = -1;
      while (model->vertices[++ndx] != fromV);
      model->vertices.erase(model->vertices.begin() + ndx);

      // Update all id's after the one removed.
      int numVerts = model->vertices.size();
      while (ndx < numVerts) {
         model->vertices[ndx]->id = model->vertices[ndx]->id - 1;
         ndx++;
      }

      // Finally delete the damn thing
      delete fromV;
   }

}