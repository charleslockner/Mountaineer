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
	virtual void sendEntityData(Entity * entity) {};
	virtual void render() {};
protected:
   void sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size, int type);
};

#endif // __SHADER_H__