
#include "shader.h"

void EntityShader::sendVertexAttribArray(unsigned int handle, unsigned int vbo, int size) {
   glEnableVertexAttribArray(handle);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glVertexAttribPointer(handle, size, GL_FLOAT, GL_FALSE, 0, 0);
}

void EntityShader::sendLargeVertexAttribArray(unsigned int handle0, unsigned int handle1,
                                              unsigned int handle2, unsigned int handle3,
                                              unsigned int vbo) {
   unsigned stride = MAX_INFLUENCES * sizeof(float);

   glEnableVertexAttribArray(handle0);
   glEnableVertexAttribArray(handle1);
   glEnableVertexAttribArray(handle2);
   glEnableVertexAttribArray(handle3);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glVertexAttribPointer(handle0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 0*sizeof(float)));
   glVertexAttribPointer(handle1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 4*sizeof(float)));
   glVertexAttribPointer(handle2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 8*sizeof(float)));
   glVertexAttribPointer(handle3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12*sizeof(float)));
}

void EntityShader::sendTexture(unsigned int handle, unsigned int id, GLenum unit) {
   glActiveTexture(unit);
   glBindTexture(GL_TEXTURE_2D, id);
   glUniform1i(handle, unit - GL_TEXTURE0);
}

// // debug tangents/bitangents
// void EntityShader::renderVertices(Camera * camera, Entity * entity) {
//    Model * model = entity->model;

//    glMatrixMode(GL_PROJECTION);
//    glLoadMatrixf(camera->getProjectionM().data());
//    glMatrixMode(GL_MODELVIEW);
//    Eigen::Matrix4f MV = camera->getViewM() * entity->generateModelM();
//    glLoadMatrixf(MV.data());

//    glBegin(GL_LINES);
//    for (int i = 0; i < model->vertices.size(); i++) {
//       printf("i = %d\n", i); // seg fault at 0 at line 47
//       // draw normal
//       glColor3f(1,0,0);
//       Eigen::Vector3f p = model->vertices[i].position;
//       glVertex3fv(p.data());
//       Eigen::Vector3f normal = model->vertices[i].normal;
//       p += normal * 0.1f;
//       glVertex3fv(p.data());

//       // draw tangent
//       glColor3f(0,1,0);
//       p = model->vertices[i].position;
//       glVertex3fv(p.data());
//       Eigen::Vector3f tangent = model->vertices[i].tangent;
//       p += tangent * 0.1f;
//       glVertex3fv(p.data());

//       // draw bitangent
//       glColor3f(0,0,1);
//       p = model->vertices[i].position;
//       glVertex3fv(p.data());
//       Eigen::Vector3f bitangent = model->vertices[i].bitangent;
//       p += bitangent * 0.1f;
//       glVertex3fv(p.data());
//    }
//    glEnd();
// }

// void EntityShader::renderBones(Camera * camera, Entity * entity) {
//    Model * model = entity->model;

//    for (int i = 0; i < model->boneCount; i++) {
//       Eigen::Vector4f p = entity->generateModelM() * entity->boneMs[i] * Eigen::Vector4f(0,0,0,1);
//       renderPoint(camera, Eigen::Vector3f(p(0), p(1), p(2)));

//       glLineWidth(3);
//       glBegin(GL_LINES);

//       glColor3f(0.8,0,0);
//       glVertex3f(p(0), p(1), p(2));
//       Eigen::Vector4f dir = entity->generateModelM() * entity->boneMs[i] * Eigen::Vector4f(1,0,0,0);
//       Eigen::Vector4f end = p + dir * 0.5f;
//       glVertex3f(end(0), end(1), end(2));

//       glColor3f(0,0.8,0);
//       glVertex3f(p(0), p(1), p(2));
//       dir = entity->generateModelM() * entity->boneMs[i] * Eigen::Vector4f(0,1,0,0);
//       end = p + dir * 0.5f;
//       glVertex3f(end(0), end(1), end(2));

//       glColor3f(0,0,0.8);
//       glVertex3f(p(0), p(1), p(2));
//       dir = entity->generateModelM() * entity->boneMs[i] * Eigen::Vector4f(0,0,1,0);
//       end = p + dir * 0.5f;
//       glVertex3f(end(0), end(1), end(2));

//       glEnd();
//       glLineWidth(1);
//    }
// }

void EntityShader::renderPoint(Camera * camera, Eigen::Vector3f p) {
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(camera->getProjectionM().data());
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(camera->getViewM().data());

   glPointSize(8);
   glBegin(GL_POINTS);
   glColor3f(0.1,0.8,0.6);
   glVertex3f(p(0), p(1), p(2));
   glEnd();
}
