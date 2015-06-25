#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "safe_gl.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "entity_rigid.h"

#include <vector>

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

LightData lightData;
Camera * camera;

double lastScreenX;
double lastScreenY;

bool keyToggles[512] = {false};
bool mouseToggle = false;

Model * cubeModel;
RigidEntity * cubeEnt;
StaticShader * shader;

float fov;

// ======================================================================== //
// ======================= INPUT CALLBACK FUNCTIONS ======================= //
// ======================================================================== //

static void error_callback(int error, const char* description) {
   fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (action == GLFW_PRESS) {
      switch (key) {
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
         case GLFW_KEY_ENTER:
            cubeEnt->bodies[0].torque = camera->getForward();
            break;
         case GLFW_KEY_RIGHT_SHIFT:
            cubeEnt->bodies[0].force = camera->getForward();
            break;
         case GLFW_KEY_T:
         case GLFW_KEY_L:
         case GLFW_KEY_C:
         case GLFW_KEY_I:
         case GLFW_KEY_M:
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_ENTER:
            cubeEnt->bodies[0].torque = Eigen::Vector3f(0,0,0);
            break;
         case GLFW_KEY_RIGHT_SHIFT:
            cubeEnt->bodies[0].force = Eigen::Vector3f(0,0,0);
            break;
         case GLFW_KEY_T:
            keyToggles[key] = !keyToggles[key];
            keyToggles[key] ?
               glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL) :
               glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
         case GLFW_KEY_L:
            keyToggles[key] = !keyToggles[key];
            keyToggles[key] ?
               glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) :
               glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
         case GLFW_KEY_C:
            keyToggles[key] = !keyToggles[key];
            keyToggles[key] ?
               glDisable(GL_CULL_FACE) :
               glEnable(GL_CULL_FACE);
            break;
         case GLFW_KEY_G:
            break;
         default:
            keyToggles[key] = false;
            break;
      }
   }
}

static Eigen::Vector2f calculateNDC(GLFWwindow * window) {
   double mouse_x, mouse_y;
   int width, height;

   glfwGetCursorPos(window, & mouse_x, & mouse_y);
   glfwGetWindowSize(window, & width, & height);

   float x_ndc = (2.0 * mouse_x) / width - 1.0;
   float y_ndc = (2.0 * mouse_y) / height - 1.0;
   return Eigen::Vector2f(x_ndc, y_ndc);
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y) {
   if (lastScreenX == 0 && lastScreenY == 0) {
      lastScreenX = x;
      lastScreenY = y;
   }

   if (!keyToggles[GLFW_KEY_T])
      camera->aim(0.001 * (lastScreenX - x), 0.001 * (y - lastScreenY), 0);

   lastScreenX = x;
   lastScreenY = y;
}

static void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
   if (action == GLFW_PRESS) {
      if (button == GLFW_MOUSE_BUTTON_1) {
         mouseToggle = true;

         // if (keyToggles[GLFW_KEY_I]) {
         //    Eigen::Vector2f ndc = calculateNDC(window);
         //    Geom::Rayf mouseRay(camera->position, camera->rayFromNDCToWorld(ndc(0), -ndc(1)));

         //    float leastDistSq = mouseRay.squaredDistToPoint(climberEnt->ikLimbs[0]->goal);
         //    int leastNdx = 0;
         //    for (int limbNdx = 1; limbNdx < 4; limbNdx++) {
         //       float distSq = mouseRay.squaredDistToPoint(climberEnt->ikLimbs[limbNdx]->goal);
         //       if (distSq < leastDistSq) {
         //          leastDistSq = distSq;
         //          leastNdx = limbNdx;
         //       }
         //    }
         //    goalIndex = leastNdx;
         // }
      }
   }
   else if (action == GLFW_RELEASE) {
      if (button == GLFW_MOUSE_BUTTON_1) {
         mouseToggle = false;
      }
   }
}

static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
   fov += 0.01 * y_offset;
   camera->setHFOV(fov);
}

// ======================================================================== //
// ========================== INITIALIZATION CODE ========================= //
// ======================================================================== //

static void setupLights() {
   lightData.lights[0].direction = Eigen::Vector3f(0.881, -0.365, 0.292);
   lightData.lights[0].color = 1.4 * Eigen::Vector3f(0.7, 0.44, 0.38);

   lightData.lights[1].direction = Eigen::Vector3f(-0.881, 0.365, -0.292);
   lightData.lights[1].color = 1.2 * Eigen::Vector3f(0.03, 0.05, 0.2);

   lightData.numLights = 2;
}

static void initialize() {
   shader = new StaticShader();
   camera = new Camera(Eigen::Vector3f(0,0,-10));
   fov = 1.0;
   camera->setHFOV(fov);
   setupLights();

   // The rigid body
   cubeModel = new Model();
   cubeModel->loadCIAB("assets/models/book.ciab");
   cubeModel->loadTexture("assets/textures/book_DIFF.png", false);
   cubeEnt = new RigidEntity(Eigen::Vector3f(0, 0, 0), cubeModel);
}

// ======================================================================== //
// ============================== UPDATE CODE ============================= //
// ======================================================================== //

static void updateCamera(GLFWwindow * window, double timePassed) {

   float distDelta = 5 * timePassed;

   if (keyToggles[GLFW_KEY_W])
      camera->moveForward(distDelta);
   if (keyToggles[GLFW_KEY_S])
      camera->moveBackward(distDelta);
   if (keyToggles[GLFW_KEY_A])
      camera->moveLeft(distDelta);
   if (keyToggles[GLFW_KEY_D])
      camera->moveRight(distDelta);
   if (keyToggles[GLFW_KEY_SPACE])
      camera->moveUp(distDelta);
   if (keyToggles[GLFW_KEY_LEFT_SHIFT])
      camera->moveDown(distDelta);

   if (keyToggles[GLFW_KEY_Q])
      camera->aim(0,0, -0.05);
   if (keyToggles[GLFW_KEY_E])
      camera->aim(0,0, 0.05);
}

static void updateEntities(GLFWwindow * window, double deltaTime) {
   cubeEnt->update(deltaTime);
}

static void draw(double deltaTime) {
   shader->render(camera, & lightData, cubeEnt);
}

static void updateLoop(GLFWwindow * window, double deltaTime) {
   updateCamera(window, deltaTime);
   updateEntities(window, deltaTime);
   draw(deltaTime);
}

// ======================================================================== //
// ================================= MAIN ================================= //
// ======================================================================== //

int main(int argc, char ** argv) {
   srand(time(NULL));

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
   glfwSetMouseButtonCallback(window, mouse_click_callback);
   glfwSetScrollCallback(window, scroll_callback);

   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearColor(0.5,0.5,0.5,1);

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   initialize(); // game code

   double timePassed = 0;
   unsigned int numFrames = 0;
   double lastTime = glfwGetTime();

   while (!glfwWindowShouldClose(window)) {
      timePassed = glfwGetTime();
      double deltaTime = timePassed - lastTime;
      lastTime = timePassed;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      updateLoop(window, deltaTime); // game code

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
