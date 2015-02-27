#ifndef __SHADER_H__
#define __SHADER_H__

#include "safe_gl.h"

#include "camera.h"
#include "light.h"
#include "entity.h"

class Entity;

// Generalized shader class for rendering entities
class EntityShader {
public:
   virtual void render(Camera * camera, LightData * lightdata, Entity * entity) {};

protected:
   typedef struct {
      unsigned int uHasNormals, uHasColors, uHasTexture, uHasNormalMap, uHasSpecularMap,
                   uModelM, uProjViewM, uCameraPosition, uLights, uTexture,
                   uNormalMap, uSpecularMap, uBoneMs,
                   aPosition, aNormal, aColor, aUV, aTangent, aBitangent,
                   aBoneIndices0, aBoneIndices1, aBoneIndices2, aBoneIndices3,
                   aBoneWeights0, aBoneWeights1, aBoneWeights2, aBoneWeights3,
                   aNumInfluences;
   } HandleTable;

   void sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size);
   void sendLargeVertexAttribArray(unsigned int handle0, unsigned int handle1,
                                   unsigned int handle2, unsigned int handle3,
                                   unsigned int vbo);
   void sendTexture(unsigned int handle, unsigned int id, GLenum texture);
};


class ForwardShader: public EntityShader {
public:
   ForwardShader();
   ~ForwardShader();
   void render(Camera * camera, LightData * lightdata, Entity * entity);

private:
   void fillHandleTable(HandleTable * table, unsigned int prog, bool animated);

   HandleTable animTable, statTable;
   unsigned int animProg, statProg;
};

#endif // __SHADER_H__
