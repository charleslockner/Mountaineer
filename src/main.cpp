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
std::vector<AnimatedEntity *> entities;

double lastScreenX;
double lastScreenY;
int goalIndex = 0;
Eigen::Vector3f goals[2];
bool playing = false;

bool keyToggles[512] = {false};

Model * guyModel;

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
         case GLFW_KEY_M:
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_Q:
            playing = !playing;
            // if (playing)
            //    entities[0]->playAnimation(0, 0, true);
            // else
            //    entities[0]->stopAnimation(0, true);
            // break;
         case GLFW_KEY_E:
            goalIndex = goalIndex == 0 ? 1 : 0;
            break;
         case GLFW_KEY_T:
            keyToggles[key] = !keyToggles[key];
            break;
         case GLFW_KEY_M:
            guyModel->loadConstraints("assets/models/guy.cns");
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

   // entities[0]->boneController->setLimbGoal(0, goals[0]);
   // entities[0]->boneController->setLimbGoal(1, goals[1]);

   // entities[0]->position += Eigen::Vector3f(0,0.5,0) * timePassed;
}

void setupLimbs() {
   // Set up the left arm
   IKLimb limb;
   // limb.reachBoneIndices.push_back(0);
   // limb.reachBoneIndices.push_back(1);
   // limb.reachBoneIndices.push_back(2);
   // limb.reachBoneIndices.push_back(3);
   limb.reachBoneIndices.push_back(9);
   limb.reachBoneIndices.push_back(10);
   limb.reachBoneIndices.push_back(11);
   limb.reachBoneIndices.push_back(12);
   limb.reachBoneIndices.push_back(13);
   // limb.baseOffset = Eigen::Vector3f(0,0,0);
   // limb.reachOffset = Eigen::Vector3f(0,0,0);
   // limb.baseGoal = Eigen::Vector3f(0,0,0);
   // limb.reachGoal = Eigen::Vector3f(0,0,0);
   guyModel->limbs.push_back(limb);
}

int main(int argc, char ** argv) {
   GLFWwindow * window = windowSetup();

   EntityShader * shader = new ForwardShader();
   camera = new Camera(Eigen::Vector3f(0,0,10), Eigen::Vector3f(0,0,-1), Eigen::Vector3f(0,1,0));
   setupLights();

   Model * chebModel = new Model();
   chebModel->loadOBJ("assets/cheb/cheb2.obj");
   chebModel->loadSkinningPIN("assets/cheb/cheb_attachment.txt");
   chebModel->loadAnimationPIN("assets/cheb/cheb_skel_walkAndSkip.txt");
   entities.push_back(new BonelessEntity(Eigen::Vector3f(-8, 0, 5), chebModel));
   entities[0]->playAnimation(0);

   Model * trexModel = new Model();
   trexModel->loadCIAB("assets/models/trex.ciab");
   trexModel->loadTexture("assets/textures/masonry.png");
   trexModel->loadNormalMap("assets/textures/masonry_normal.png");
   entities.push_back(new BonifiedEntity(Eigen::Vector3f(10, 0, -15), trexModel));
   entities[1]->playAnimation(0);

   guyModel = new Model();
   guyModel->loadCIAB("assets/models/guy.ciab");
   guyModel->loadTexture("assets/textures/guy_tex.bmp");
   guyModel->loadConstraints("assets/models/guy.cns");
   setupLimbs();
   entities.push_back(new IKEntity(Eigen::Vector3f(0, 0, 0), guyModel));
   entities[2]->playAnimation(0);


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

      // if (keyToggles[GLFW_KEY_K]) {
      //    shader->renderVertices(camera, entities[1]);
      //    shader->renderBones(camera, entities[0]);
      // }
      // shader->renderPoint(camera, goals[goalIndex]);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
