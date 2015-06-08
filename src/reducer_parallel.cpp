#include "reducer.h"
#include "model.h"

#include <assert.h>
#include <algorithm>
#include <iostream>

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/tbb_allocator.h"
#include "tbb/atomic.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_sort.h"
#include "tbb/concurrent_vector.h"
#include "tbb/mutex.h"

// ========================================================== //
// ==================== STATIC FUNCTIONS ==================== //
// ========================================================== //

static void makeNeighbors(Vertex * a, Vertex * b) {
   a->neighbors.push_back(b);
   b->neighbors.push_back(a);
}

static void removeNeighborReferences(Vertex * v) {
   int numNeighs = v->neighbors.size();
   for (int i = 0; i < numNeighs; i++) {
      Vertex * n = v->neighbors[i];
      n->neighbors.erase(find(n->neighbors.begin(), n->neighbors.end(), v));
   }
}

static void removeFaceFromVertexReferences(Face * f) {
   f->vertices[0]->faces.erase(find(f->vertices[0]->faces.begin(), f->vertices[0]->faces.end(), f));
   f->vertices[1]->faces.erase(find(f->vertices[1]->faces.begin(), f->vertices[1]->faces.end(), f));
   f->vertices[2]->faces.erase(find(f->vertices[2]->faces.begin(), f->vertices[2]->faces.end(), f));
}

static bool areNeighbors(Vertex * a, Vertex * b) {
   int aNumNeighs = a->neighbors.size();
   int bNumNeighs = b->neighbors.size();

   if (aNumNeighs < bNumNeighs) {
      for (int i = 0; i < aNumNeighs; i++) {
         if (a->neighbors[i] == b) {
            return true;
         }
      }
   }
   else {
      for (int i = 0; i < bNumNeighs; i++) {
         if (b->neighbors[i] == a) {
            return true;
         }
      }
   }

   return false;
}

// ========================================================== //
// ==================== PUBLIC FUNCTIONS ==================== //
// ========================================================== //

namespace MR {

   tbb::mutex M1;
   tbb::mutex M2;

   void Collapse(Model * model, Vertex * fromV, Vertex * toV) {
      // assert(areNeighbors(fromV, toV));
      M1.lock();

      // Remove shared faces
      for (int i = 0; i < fromV->faces.size(); i++) {
         for (int j = 0; j < toV->faces.size(); j++) {
            if (fromV->faces[i] == toV->faces[j]) {
               Face * f = fromV->faces[i];
               removeFaceFromVertexReferences(f);
               model->faces.erase(find(model->faces.begin(), model->faces.end(), f));
               delete(f);
               i--;
               j--;
            }
         }
      }

      int numFromFaces = fromV->faces.size();
      for (int i = 0; i < fromV->faces.size(); i++) {
         // Update fromV's faces' vertex references
         for (int j = 0; j < NUM_FACE_EDGES; j++) {
            if (fromV->faces[i]->vertices[j] == fromV) {
               fromV->faces[i]->vertices[j] = toV;
               break;
            }
         }

         // Add each face of fromV to toV's face references
         toV->faces.push_back(fromV->faces[i]);
      }

      M1.unlock();
      M2.lock();

      // Update neighbor references
      int numFromNeighs = fromV->neighbors.size();
      for (int i = 0; i < numFromNeighs; i++) {
         Vertex * fromNeighV = fromV->neighbors[i];
         // Make fromNeighV and toV neighbors if not already
         if (!areNeighbors(fromNeighV, toV) && fromNeighV != toV) {
            makeNeighbors(fromNeighV, toV);
         }
      }

      // Remove all references to fromV from fromV's neighbors
      removeNeighborReferences(fromV);

      // Remove fromV from model's vertex list
      std::vector<Vertex *>::iterator it = find(model->vertices.begin(), model->vertices.end(), fromV);
      model->vertices.erase(it);

      // Update all id's after the one removed.
      int delNdx = it - model->vertices.begin();
      int numVerts = model->vertices.size() - delNdx;

      for (int i = 0; i < numVerts; i++) {
         model->vertices[delNdx + i]->index--;
      }

      // Finally delete the damn thing
      delete fromV;

      M2.unlock();
   }

}