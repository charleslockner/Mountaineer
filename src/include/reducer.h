
#ifndef __REDUCER_H__
#define __REDUCER_H__

struct Model;
struct Vertex;

namespace MR {

   void Collapse(Model * model, Vertex * from, Vertex * to);

}

#endif // __REDUCER_H__