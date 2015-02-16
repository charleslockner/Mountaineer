#ifndef __SHADER_H__
#define __SHADER_H__

#define USE_SMART_USE_PROGRAM
#define USE_SAFE_GL
#include "safe_gl.h"

#include "camera.h"
#include "world.h"
#include "model.h"
#include "entity.h"

class Entity;


// Generalized shader class for rendering entities
class EntityShader {
public:
	virtual void sendCameraData(Camera * camera) {};
   virtual void sendWorldData(World * world) {};
	virtual void sendModelData(Model * model) {};
	virtual void renderEntity(Entity * entity) {};

protected:
   void sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size);
   void sendLargeVertexAttribArray(unsigned int handle0, unsigned int handle1,
                                   unsigned int handle2, unsigned int handle3,
                                   unsigned int vbo, int numAttribs);
   void sendTexture(unsigned int handle, unsigned int id, GLenum texture);
};


class ForwardShader: public EntityShader {
public:
   ForwardShader();
   ~ForwardShader();

   void sendCameraData(Camera * camera);
   void sendWorldData(World * world);
   void sendModelData(Model * model);
   void renderEntity(Entity * entity);

private:
   void setupHandles();

   unsigned int h_uHasNormals;
   unsigned int h_uHasColors;
   unsigned int h_uHasTextures;
   unsigned int h_uHasTansAndBitans;
   unsigned int h_uHasAnimations;

   unsigned int h_uModelMatrix;
   unsigned int h_uProjViewMatrix;
   unsigned int h_uCameraPosition;
   unsigned int h_uLights;
   unsigned int h_uTexture;
   unsigned int h_uBoneMatrices;

   unsigned int h_aVertexPosition;
   unsigned int h_aVertexNormal;
   unsigned int h_aVertexColor;
   unsigned int h_aVertexUV;
   unsigned int h_aBoneIndices;
   unsigned int h_aBoneWeights;

   unsigned int h_aBoneIndices0;
   unsigned int h_aBoneIndices1;
   unsigned int h_aBoneIndices2;
   unsigned int h_aBoneIndices3;
   unsigned int h_aBoneWeights0;
   unsigned int h_aBoneWeights1;
   unsigned int h_aBoneWeights2;
   unsigned int h_aBoneWeights3;
   unsigned int h_aNumInfluences;

   unsigned int program;
   unsigned int indexCount;
   glm::mat4 projectionMatrix;
};

#endif // __SHADER_H__