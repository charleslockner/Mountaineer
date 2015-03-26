#include <stdio.h>
#include <stdlib.h>

#include "safe_gl.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "entity.h"

#include <vector>

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

#define CAMERA_SPEED 5.0

LightData lightData;
Camera * camera;
std::vector<Entity *> entities;

double lastScreenX;
double lastScreenY;
int goalIndex = 0;
Eigen::Vector3f goals[2];
bool playing = false;

bool keyToggles[512] = {false};

static void error_callback(int error, const char* description) {
   fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (action == GLFW_PRESS) {
      switch (key) {
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
         case GLFW_KEY_Q:
         case GLFW_KEY_E:
         case GLFW_KEY_T:
         case GLFW_KEY_L:
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_Q:
            playing = !playing;
            if (playing)
               entities[0]->boneController->playAnimation(0, 0, true);
            else
               entities[0]->boneController->stopAnimation(0, true);
            break;
         case GLFW_KEY_E:
            goalIndex = goalIndex == 0 ? 1 : 0;
            break;
         case GLFW_KEY_T:
            keyToggles[key] = !keyToggles[key];
            break;
         case GLFW_KEY_L:
            keyToggles[key] = !keyToggles[key];
            keyToggles[key] ?
               glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) :
               glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
         default:
            keyToggles[key] = false;
            break;
      }
   }
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y) {
   if (lastScreenX == 0 && lastScreenY == 0) {
      lastScreenX = x;
      lastScreenY = y;
   }

   camera->aim(x - lastScreenX, y - lastScreenY);
   lastScreenX = x;
   lastScreenY = y;
}

void setupLights() {
   lightData.lights[0].position = Eigen::Vector3f(18.0, 10.0, 10.0);
   lightData.lights[0].direction = Eigen::Vector3f(1.0, 0.0, 0.0);
   lightData.lights[0].color = Eigen::Vector3f(1.0, 1.0, 1.0);
   lightData.lights[0].strength = 250;
   lightData.lights[0].attenuation = 50.0;
   lightData.lights[0].spread = 15;
   lightData.numLights = 1;
}


GLFWwindow * windowSetup() {
   GLFWwindow * window;
   glfwSetErrorCallback(error_callback);

   if (!glfwInit())
      exit(EXIT_FAILURE);

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

   window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Physical Animation", NULL, NULL);
   if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   glfwMakeContextCurrent(window);
   glfwSwapBuffers(window);

   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, cursor_pos_callback);
   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearColor(0.4,0.2,0.2,1.0);

   return window;
}

void setupLimbs(Model * guyModel) {
   IKJoint joint;
   std::vector<short> boneIndices;
   IKSolver * solver;
   guyModel->limbSolvers = std::vector<IKSolver*>(2);

   // left arm
   guyModel->bones[9].limbIndex = 0;

   joint.axis = Eigen::Vector3f(1,0,0);
   joint.minAngle = -M_PI/2;
   joint.maxAngle = M_PI/2;
   guyModel->bones[10].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,1,0);
   joint.minAngle = -M_PI/3;
   joint.maxAngle = M_PI/2;
   guyModel->bones[10].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,0,1);
   joint.minAngle = -M_PI/2;
   joint.maxAngle = M_PI/3;
   guyModel->bones[10].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,1,0);
   joint.minAngle = -M_PI/2;
   joint.maxAngle = M_PI/3;
   guyModel->bones[11].joints.push_back(joint);

   boneIndices.push_back(9);
   boneIndices.push_back(10);
   boneIndices.push_back(11);
   boneIndices.push_back(12);
   boneIndices.push_back(13);

   guyModel->limbSolvers[0] = new IKSolver(guyModel, boneIndices);
   boneIndices.clear();

   // right arm
   guyModel->bones[15].limbIndex = 1;

   joint.axis = Eigen::Vector3f(1,0,0);
   joint.minAngle = -M_PI;
   joint.maxAngle = M_PI;
   guyModel->bones[16].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,1,0);
   joint.minAngle = -M_PI;
   joint.maxAngle = M_PI;
   guyModel->bones[16].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(1,0,0);
   joint.minAngle = -M_PI;
   joint.maxAngle = M_PI;
   guyModel->bones[17].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,1,0);
   joint.minAngle = -M_PI;
   joint.maxAngle = M_PI;
   guyModel->bones[17].joints.push_back(joint);
   joint.axis = Eigen::Vector3f(0,1,0);
   joint.minAngle = -M_PI/2;
   joint.maxAngle = M_PI/2;
   guyModel->bones[18].joints.push_back(joint);

   boneIndices.push_back(15);
   boneIndices.push_back(16);
   boneIndices.push_back(17);
   boneIndices.push_back(18);
   boneIndices.push_back(19);

   guyModel->limbSolvers[1] = new IKSolver(guyModel, boneIndices);
   boneIndices.clear();

}

void updateCameraPosition(double timePassed) {
   float distTraveled = CAMERA_SPEED * timePassed;

   if (keyToggles[GLFW_KEY_W])
      camera->moveForward(distTraveled);
   if (keyToggles[GLFW_KEY_S])
      camera->moveBackward(distTraveled);
   if (keyToggles[GLFW_KEY_A])
      camera->moveLeft(distTraveled);
   if (keyToggles[GLFW_KEY_D])
      camera->moveRight(distTraveled);
   if (keyToggles[GLFW_KEY_SPACE])
      camera->moveUp(distTraveled);
   if (keyToggles[GLFW_KEY_LEFT_SHIFT])
      camera->moveDown(distTraveled);
   if (keyToggles[GLFW_KEY_T])
      camera->lookAt(entities[0]->position);
}

void updateWorld(double timePassed) {
   double speed = 3.0f;

   if (keyToggles[GLFW_KEY_X]) {
      if (keyToggles[GLFW_KEY_LEFT_CONTROL])
         goals[goalIndex](0) = goals[goalIndex](0) - timePassed * speed;
      else
         goals[goalIndex](0) = goals[goalIndex](0) + timePassed * speed;
   }
   if (keyToggles[GLFW_KEY_Y]) {
      if (keyToggles[GLFW_KEY_LEFT_CONTROL])
         goals[goalIndex](1) = goals[goalIndex](1) - timePassed * speed;
      else
         goals[goalIndex](1) = goals[goalIndex](1) + timePassed * speed;
   }
   if (keyToggles[GLFW_KEY_Z]) {
      if (keyToggles[GLFW_KEY_LEFT_CONTROL])
         goals[goalIndex](2) = goals[goalIndex](2) - timePassed * speed;
      else
         goals[goalIndex](2) = goals[goalIndex](2) + timePassed * speed;
   }

   entities[0]->boneController->setLimbGoal(0, goals[0]);
   entities[0]->boneController->setLimbGoal(1, goals[1]);

   // entities[0]->position += Eigen::Vector3f(0,0.5,0) * timePassed;
}

int main(int argc, char ** argv) {
   GLFWwindow * window = windowSetup();

   EntityShader * shader = new ForwardShader();
   camera = new Camera(Eigen::Vector3f(0,0,10), Eigen::Vector3f(0,0,-1), Eigen::Vector3f(0,1,0));
   setupLights();

   Model * guyModel = new Model();
   guyModel->loadCIAB("assets/models/guy.ciab");
   guyModel->loadTexture("assets/textures/guy_tex.bmp");
   // guyModel->loadNormalMap("assets/textures/masonry_normal.png");
   setupLimbs(guyModel);
   entities.push_back(new Entity(Eigen::Vector3f(0, 0, 0), guyModel));

   Model * trexModel = new Model();
   trexModel->loadCIAB("assets/models/trex.ciab");
   trexModel->loadTexture("assets/textures/masonry.png");
   trexModel->loadNormalMap("assets/textures/masonry_normal.png");
   entities.push_back(new Entity(Eigen::Vector3f(10, 0, -15), trexModel));
   entities[1]->boneController->playAnimation(0, 0, true);

   Model * chebModel = new Model();
   chebModel->loadOBJ("assets/cheb/cheb2.obj");
   chebModel->loadSkinningPIN("assets/cheb/cheb_attachment.txt");
   chebModel->loadAnimationPIN("assets/cheb/cheb_skel_walkAndSkip.txt");
   entities.push_back(new Entity(Eigen::Vector3f(-8, 0, 5), chebModel));
   entities[2]->boneController->playAnimation(0, 0, false);

   double timePassed = 0;
   unsigned int numFrames = 0;
   double lastTime = glfwGetTime();

   while (!glfwWindowShouldClose(window)) {
      timePassed = glfwGetTime();
      double deltaTime = timePassed - lastTime;
      lastTime = timePassed;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      updateCameraPosition(deltaTime);
      updateWorld(deltaTime);

      for (int i = 0; i < entities.size(); i++)
         entities[i]->update(deltaTime);
      for (int i = 0; i < entities.size(); i++)
         shader->render(camera, & lightData, entities[i]);

      if (keyToggles[GLFW_KEY_K]) {
         shader->renderVertices(camera, entities[1]);
         shader->renderBones(camera, entities[1]);
         shader->renderBones(camera, entities[0]);
      }
      shader->renderPoint(camera, goals[goalIndex]);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
