#include <OpenGL/gl.h>
#include <math.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "forward_shader.h"
#include "shader_builder.h"

#define HFOV   M_PI/4.0
#define ASPECT 4.0 / 3.0
#define NEAR   0.1
#define FAR    1000.0

ForwardShader::ForwardShader() {
   program = SB_buildFromPaths("shaders/forward.vert.glsl", "shaders/forward.frag.glsl");
   setupHandles();
   projectionMatrix = glm::perspective(HFOV, ASPECT, NEAR, FAR);
};

ForwardShader::~ForwardShader() {};

void ForwardShader::setupHandles() {
   h_uModelMatrix = glGetUniformLocation(program, "uModelMatrix");
   h_uProjViewMatrix = glGetUniformLocation(program, "uProjViewMatrix");
   h_uCameraPosition = glGetUniformLocation(program, "uCameraPosition");
   // h_uLights = glGetUniformLocation(program, "uLights");
   // h_uTexture = glGetUniformLocation(program, "uTexture");
   h_uBoneTransforms = glGetUniformLocation(program, "uBoneTransforms");

   h_aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
   h_aVertexNormal = glGetAttribLocation(program, "aVertexNormal");
   // h_aVertexColor = glGetAttribLocation(program, "aVertexColor");
   // h_aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
   h_aBoneIndices = glGetAttribLocation(program, "aBoneIndices");
   h_aBoneWeights = glGetAttribLocation(program, "aBoneWeights");
}

void ForwardShader::sendWorldData(World * world) {
   glUseProgram(program);

   glUniform3fv(h_uLights, 4*world->numLights, (GLfloat*)(world->lights));
}

void ForwardShader::sendCameraData(Camera * camera) {
   glUseProgram(program);

   glm::vec3 target = camera->position + camera->direction;
   glm::mat4 viewMatrix = glm::lookAt(camera->position, target, camera->up);
   glm::mat4 pvMatrix = projectionMatrix * viewMatrix;
   glUniformMatrix4fv(h_uProjViewMatrix, 1, GL_FALSE, glm::value_ptr(pvMatrix));

   glUniform3fv(h_uCameraPosition, 1, glm::value_ptr(camera->position));
}

void ForwardShader::sendModelData(Model * model) {
   glUseProgram(program);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, model->tbo);
   glUniform1i(h_uTexture, 0);

   sendVertexAttribArray(h_aVertexPosition, model->vbo, 3, GL_FLOAT);
   sendVertexAttribArray(h_aVertexNormal, model->nbo, 3, GL_FLOAT);
   // sendVertexAttribArray(h_aVertexColor, model->cbo, 3, GL_FLOAT);
   // sendVertexAttribArray(h_aTextureCoord, model->uvbo, 2, GL_FLOAT);
   sendVertexAttribArray(h_aBoneIndices, model->bibo, 4, GL_FLOAT);
   sendVertexAttribArray(h_aBoneWeights, model->bwbo, 4, GL_FLOAT);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ibo);
   indexCount = model->indexCount;
}

void ForwardShader::sendEntityData(Entity * entity) {
   glUseProgram(program);

   glm::mat4 transM = glm::translate(glm::mat4(1.0f), entity->position);
   glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), entity->scale);
   glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), entity->rotation, glm::vec3(0,1,0));
   glm::mat4 modelM = transM * scaleM * rotateM;
   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(modelM));

   glUniformMatrix4fv(h_uBoneTransforms, MAX_BONES, GL_FALSE, (GLfloat *)(entity->boneTransforms));
}

void ForwardShader::render() {
   glUseProgram(program);

   glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

