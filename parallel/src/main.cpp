#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "safe_gl.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "entity.h"
#include "terrain.h"

#include <vector>

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

LightData lightData;
Camera * camera;
std::vector<ModelEntity *> staticEntities;

double lastScreenX;
double lastScreenY;
int goalIndex = 0;
Eigen::Vector3f goals[4];

bool keyToggles[512] = {false};

ModelEntity * skyEnt, * terrainEnt;
EntityShader * entShader;
TextureShader * texShader;
TerrainGenerator * terrainGenerator;

Eigen::Vector3f mouseDirection;
Eigen::Vector3f camGoal;
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
         case GLFW_KEY_T:
         case GLFW_KEY_L:
         case GLFW_KEY_C:
         case GLFW_KEY_I:
         case GLFW_KEY_M:
            break;
         case GLFW_KEY_G:
            printf("pressed g\n");
            terrainGenerator->UpdateMesh(camera->position, 10);
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
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
   lightData.lights[0].color = Eigen::Vector3f(0.7, 0.44, 0.38);

   lightData.lights[1].direction = Eigen::Vector3f(-0.881, 0.365, -0.292);
   lightData.lights[1].color = Eigen::Vector3f(0.03, 0.05, 0.2);

   lightData.numLights = 2;
}

static void initialize() {
   entShader = new ForwardShader();
   texShader = new TextureShader();
   camera = new Camera(Eigen::Vector3f(-100,0,200));
   camera->rotation = Eigen::Quaternionf(0.5, 0, 1, 0);
   fov = 1.0;
   camera->setHFOV(fov);
   setupLights();

   // Skybox
   Model * skyModel = new Model();
   skyModel->loadCIAB("assets/models/skybox.ciab");
   skyModel->loadTexture("assets/textures/night.png", false);
   skyEnt = new ModelEntity(Eigen::Vector3f(0,-250,0),
                            Eigen::Quaternionf(1,0,0,0),
                            Eigen::Vector3f(1000,1000,1000),
                            skyModel);

   // Terrain Stuff
   terrainGenerator = new TerrainGenerator();
   Model * terrainModel = terrainGenerator->GenerateModel();
   terrainModel->loadTexture("assets/textures/rock.png", true);
   terrainModel->loadNormalMap("assets/textures/rock_NORM.png", true);
   terrainModel->loadSpecularMap("assets/textures/rock_SPEC.png", true);
   terrainEnt = new ModelEntity(Eigen::Vector3f(0, 0, 0), terrainModel);
}

// ======================================================================== //
// ============================== UPDATE CODE ============================= //
// ======================================================================== //

// static void updateCamera(GLFWwindow * window, double timePassed) {
//    float distTraveled = 20 * timePassed;

//    if (keyToggles[GLFW_KEY_W])
//       camera->moveForward(distTraveled);
//    if (keyToggles[GLFW_KEY_S])
//       camera->moveBackward(distTraveled);
//    if (keyToggles[GLFW_KEY_A])
//       camera->moveLeft(distTraveled);
//    if (keyToggles[GLFW_KEY_D])
//       camera->moveRight(distTraveled);
//    if (keyToggles[GLFW_KEY_SPACE])
//       camera->moveUp(distTraveled);
//    if (keyToggles[GLFW_KEY_LEFT_SHIFT])
//       camera->moveDown(distTraveled);

//    if (keyToggles[GLFW_KEY_Q])
//       camera->aim(0,0, -0.05);
//    if (keyToggles[GLFW_KEY_E])
//       camera->aim(0,0, 0.05);

//    camGoal = camera->position + 10 * camera->getForward();
// }

static void draw() {
   texShader->render(camera, & lightData, skyEnt);
   entShader->render(camera, & lightData, terrainEnt);

   if (keyToggles[GLFW_KEY_K]) {
      entShader->renderVertices(camera, terrainEnt);
      entShader->renderPaths(camera, terrainGenerator);
      entShader->renderPoint(camera, camGoal);}

}

static void update(GLFWwindow * window, float radius) {
   // updateCamera(window, timePassed);

   skyEnt->position = camera->position + Eigen::Vector3f(0, -500, 0);
   terrainGenerator->UpdateMesh(Eigen::Vector3f(0,0,0), radius);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   draw();
   glfwSwapBuffers(window);
   glfwPollEvents();

}

double lastTime;

double timeSinceLast() {
   double timePassed = glfwGetTime();
   double deltaTime = timePassed - lastTime;
   lastTime = timePassed;
   return deltaTime;
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
   glClearColor(0.2,0.2,0.2,1);

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   initialize(); // game code

   double totalTime = 0;
   lastTime = glfwGetTime();

   for (int i = 0; i < 200; i++)
      update(window, 1000000000); // game code

   double time = timeSinceLast();
   totalTime += time;
   printf("First loop time = %0.3f\n", time);

   for (int i = 0; i < 200; i++)
      update(window, 0); // game code
   
   time = timeSinceLast();
   totalTime += time;
   printf("Second loop time = %0.3f\n", time);
   printf("Total time = %0.3f\n", totalTime);

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
