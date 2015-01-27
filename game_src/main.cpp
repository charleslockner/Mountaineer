#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "forward_shader.h"
#include "model_builder.h"

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

#define CAMERA_SPEED 5.0

World worldData;
Camera * camera;
Model * cubeModel;
Entity * cubeEnt;

EntityShader * shader;

bool forwardHeld = false;
bool backwardHeld = false;
bool leftHeld = false;
bool rightHeld = false;
bool upHeld = false;
bool downHeld = false;
double lastScreenX;
double lastScreenY;

static void error_callback(int error, const char* description) {
   fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (action == GLFW_PRESS) {
      switch (key) {
         case GLFW_KEY_W:
         case GLFW_KEY_UP:
            forwardHeld = true;
            break;
         case GLFW_KEY_S:
         case GLFW_KEY_DOWN:
            backwardHeld = true;
            break;
         case GLFW_KEY_A:
         case GLFW_KEY_LEFT:
            leftHeld = true;
            break;
         case GLFW_KEY_D:
         case GLFW_KEY_RIGHT:
            rightHeld = true;
            break;
         case GLFW_KEY_SPACE:
            upHeld = true;
            break;
         case GLFW_KEY_LEFT_SHIFT:
            downHeld = true;
            break;
         case GLFW_KEY_T:
            camera->lookAt(cubeEnt->position);
            break;
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
         default:
            // WE DON'T CARE
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_W:
         case GLFW_KEY_UP:
            forwardHeld = false;
            break;
         case GLFW_KEY_S:
         case GLFW_KEY_DOWN:
            backwardHeld = false;
            break;
         case GLFW_KEY_A:
         case GLFW_KEY_LEFT:
            leftHeld = false;
            break;
         case GLFW_KEY_D:
         case GLFW_KEY_RIGHT:
            rightHeld = false;
            break;
         case GLFW_KEY_SPACE:
            upHeld = false;
            break;
         case GLFW_KEY_LEFT_SHIFT:
            downHeld = false;
            break;
         default:
            // WE DON'T CARE
            break;
      }
   }
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y) {
   camera->aim(x - lastScreenX, y - lastScreenY);
   lastScreenX = x;
   lastScreenY = y;
}

void setupWorldData() {
   worldData.lights[0].position = glm::vec3(0.0, 0.0, -1.0);
   worldData.lights[0].direction = glm::vec3(1.0, 0.0, 0.0);
   worldData.lights[0].color = glm::vec3(1.0, 1.0, 1.0);
   worldData.lights[0].strength = 50;
   worldData.lights[0].attenuation = 50.0;
   worldData.lights[0].spread = 15;
   worldData.numLights = 1;
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

   if (forwardHeld)
      camera->moveForward(distTraveled);
   if (backwardHeld)
      camera->moveBackward(distTraveled);
   if (leftHeld)
      camera->moveLeft(distTraveled);
   if (rightHeld)
      camera->moveRight(distTraveled);
   if (upHeld)
      camera->moveUp(distTraveled);
   if (downHeld)
      camera->moveDown(distTraveled);
}

int main(void) {
   GLFWwindow * window = windowSetup();

   shader = new ForwardShader();
   glfwGetCursorPos(window, & lastScreenX, & lastScreenY);

   camera = new Camera(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,1,0));
   setupWorldData();
   cubeModel = MB_build("assets/models/trex.ciab",
                        "assets/textures/stones.bmp");
   cubeEnt = new Entity(glm::vec3(0,0,-5), cubeModel);


   shader->sendCameraData(camera);
   shader->sendWorldData(&worldData);
   shader->sendModelData(cubeModel);

   double timePassed = 0;
   unsigned int numFrames = 0;
   double lastTime = glfwGetTime();

   while (!glfwWindowShouldClose(window)) {
      timePassed = glfwGetTime();
      double deltaTime = timePassed - lastTime;
      lastTime = timePassed;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      updateCameraPosition(deltaTime);

      shader->sendCameraData(camera);
      cubeEnt->draw(shader, timePassed);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
