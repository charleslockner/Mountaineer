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

bool keyToggles[512] = {false};

static void error_callback(int error, const char* description) {
   fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (action == GLFW_PRESS) {
      switch (key) {
         case GLFW_KEY_T:
            camera->lookAt(entities[0]->position);
            break;
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
         case GLFW_KEY_L:
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_L:
            keyToggles[key] = !keyToggles[key];
            if(keyToggles[key])
               glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
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
}

int main(int argc, char ** argv) {
   GLFWwindow * window = windowSetup();

   EntityShader * shader = new ForwardShader();
   camera = new Camera(Eigen::Vector3f(10,0,0), Eigen::Vector3f(0,0,-1), Eigen::Vector3f(0,1,0));
   setupLights();

   Model * model = new Model();
   model->loadCIAB("assets/models/trex.ciab");
   // model->loadTexture("assets/textures/masonry.png");
   // model->loadNormalMap("assets/textures/masonry_normal.png");
   // model->loadNormalMap("assets/textures/normal.jpg");
   // printf("hasNormalMap %d\n", model->hasNormalMap);

   entities.push_back(new Entity(Eigen::Vector3f(0, 0, 0), model));
   // camera->lookAt(entities[0]->position);

   double timePassed = 0;
   unsigned int numFrames = 0;
   double lastTime = glfwGetTime();

   while (!glfwWindowShouldClose(window)) {
      timePassed = glfwGetTime();
      double deltaTime = timePassed - lastTime;
      lastTime = timePassed;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      updateCameraPosition(deltaTime);

      for (int i = 0; i < entities.size(); i++)
         entities[i]->update(deltaTime);
      for (int i = 0; i < entities.size(); i++)
         shader->render(camera, & lightData, entities[i]);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
