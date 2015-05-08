#ifndef __SHADER_H__
#define __SHADER_H__

#include "safe_gl.h"

#include "camera.h"
#include "light.h"
#include "entity.h"

class Entity;
class AnimatedEntity;
class BonifiedEntity;

// Generalized shader class for rendering entities
class EntityShader {
public:
   virtual void render(Camera * camera, LightData * lightdata, Entity * entity) {};

   // Debug Functions
   void renderVertices(Camera * camera, Entity * entity);
   void renderBones(Camera * camera, BonifiedEntity * entity);
   void renderPoint(Camera * camera, Eigen::Vector3f p);

protected:
   typedef struct {
      unsigned int uHasNormals, uHasColors, uHasTexture, uHasNormalMap, uHasSpecularMap,
                   uModelM, uProjViewM, uCameraPosition, uLights, uTexture,
                   uNormalMap, uSpecularMap, uAnimMs,
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

   HandleTable animTable, statTable;
   unsigned int animProg, statProg;
};

class ForwardShader: public EntityShader {
public:
   ForwardShader();
   ~ForwardShader();
   void render(Camera * camera, LightData * lightdata, Entity * entity);

private:
   void fillHandleTable(HandleTable * table, unsigned int prog, bool animated);
};

class TextureShader: public EntityShader {
public:
   TextureShader();
   ~TextureShader();
   void render(Camera * camera, LightData * lightdata, Entity * entity);

protected:
   unsigned int h_uProjViewModelM, h_uTexture;
   unsigned int h_aPosition, h_aUV;
   unsigned int program;
};

#endif // __SHADER_H__
