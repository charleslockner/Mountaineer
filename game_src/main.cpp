#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "model.h"

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

#define CAMERA_SPEED 5.0

World world;
Camera * camera;
Model * model;
Entity * entity;

EntityShader * shader;

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
            camera->lookAt(entity->position);
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

void setupWorldData() {
    world.lights[0].position = glm::vec3(10.0, 10.0, -10.0);
    world.lights[0].direction = glm::vec3(1.0, 0.0, 0.0);
    world.lights[0].color = glm::vec3(1.0, 1.0, 1.0);
    world.lights[0].strength = 200;
    world.lights[0].attenuation = 50.0;
    world.lights[0].spread = 15;
    world.numLights = 1;
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

int main(void) {
   GLFWwindow * window = windowSetup();

   shader = new ForwardShader();

   camera = new Camera(glm::vec3(0,0,2), glm::vec3(0,0,-1), glm::vec3(0,1,0));
   setupWorldData();

   model = new Model();
   // model->loadCIAB("assets/models/guy.ciab");
   // model->loadTexture("assets/textures/guy_tex.bmp");
   model->loadOBJ("assets/cheb/cheb2.obj");
   model->loadSkinningPIN("assets/cheb/cheb_attachment.txt");
   model->loadAnimationPIN("assets/cheb/cheb_skel_walkAndSkip.txt");

   entity = new Entity(glm::vec3(0,0,0), model);

   camera->lookAt(entity->position);

   shader->sendCameraData(camera);
   shader->sendWorldData(& world);
   shader->sendModelData(model);

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
      entity->draw(shader, deltaTime);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
