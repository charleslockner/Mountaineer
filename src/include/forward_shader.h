#ifndef __FORWARD_SHADER__
#define __FORWARD_SHADER__

#include "shader.h"

class ForwardShader: public EntityShader {
public:
	ForwardShader();
	~ForwardShader();

	void sendCameraData(Camera * camera);
   void sendWorldData(World * world);
	void sendModelData(Model * model);
	void sendEntityData(Entity * entity);
	void render();

private:
	void setupHandles();

	unsigned int h_uModelMatrix;
   unsigned int h_uProjViewMatrix;
   unsigned int h_uCameraPosition;
   unsigned int h_uLights;
   unsigned int h_uTexture;
   unsigned int h_uBoneTransforms;

   unsigned int h_aVertexPosition;
   unsigned int h_aVertexColor;
   unsigned int h_aVertexNormal;
   unsigned int h_aTextureCoord;
   unsigned int h_aBoneIndices;
   unsigned int h_aBoneWeights;

   unsigned int program;
   unsigned int indexCount;
   glm::mat4 projectionMatrix;
};

#endif /* __FORWARD_SHADER__ */