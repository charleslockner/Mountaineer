#ifndef __SHADER_H__
#define __SHADER_H__

#include "safe_gl.h"

#include "camera.h"
#include "light.h"
#include "entity.h"
#include "terrain.h"

class Entity;
class StaticEntity;
class AnimatedEntity;
class SkinnedEntity;

// Generalized shader class for rendering entities
class EntityShader {
public:
   virtual void render(Camera * camera, LightData * lightdata, StaticEntity * entity) {};

   // Debug Functions
   void renderVertices(Camera * camera, StaticEntity * entity);
   void renderBones(Camera * camera, SkinnedEntity * entity);
   void renderPoint(Camera * camera, Eigen::Vector3f p);
   void renderPaths(Camera * camera, TerrainGenerator * tg);

protected:
   void sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size);
   void sendLargeVertexAttribArray(unsigned int handle0, unsigned int handle1,
                                   unsigned int handle2, unsigned int handle3,
                                   unsigned int vbo);
   void sendTexture(unsigned int handle, unsigned int id, GLenum texture);

   unsigned int program;
};


class StaticShader: public EntityShader {
public:
   StaticShader();
   ~StaticShader();
   void render(Camera * camera, LightData * lightdata, StaticEntity * entity);

protected:
   unsigned int h_uHasNormals, h_uHasColors, h_uHasTexture, h_uHasNormalMap, h_uHasSpecularMap;
   unsigned int h_uModelM, h_uProjViewM, h_uCameraPosition, h_uLights, h_uTexture;
   unsigned int h_uNormalMap, h_uSpecularMap;
   unsigned int h_aPosition, h_aColor, h_aNormal, h_aTangent, h_aBitangent, h_aUV;
};


class TextureShader: public EntityShader {
public:
   TextureShader();
   ~TextureShader();
   void render(Camera * camera, LightData * lightdata, StaticEntity * entity);

protected:
   unsigned int h_uProjViewModelM, h_uTexture;
   unsigned int h_aPosition, h_aUV;
};


class AnimatedShader: public EntityShader {
public:
   AnimatedShader();
   ~AnimatedShader();
   void render(Camera * camera, LightData * lightdata, AnimatedEntity * entity);

protected:
   unsigned int h_uHasNormals, h_uHasColors, h_uHasTexture, h_uHasNormalMap, h_uHasSpecularMap;
   unsigned int h_uModelM, h_uProjViewM, h_uCameraPosition, h_uLights, h_uTexture;
   unsigned int h_uNormalMap, h_uSpecularMap;
   unsigned int h_uAnimMs;
   unsigned int h_aPosition, h_aColor, h_aNormal, h_aTangent, h_aBitangent, h_aUV;
   unsigned int h_aBoneIndices0, h_aBoneIndices1, h_aBoneIndices2, h_aBoneIndices3;
   unsigned int h_aBoneWeights0, h_aBoneWeights1, h_aBoneWeights2, h_aBoneWeights3;
   unsigned int h_aNumInfluences;
};



#endif // __SHADER_H__
