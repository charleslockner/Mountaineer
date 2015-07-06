#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "safe_gl.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "entity.h"
#include "entity_ik.h"
#include "terrain.h"

#include <vector>

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

LightData lightData;
Camera * camera;
std::vector<StaticEntity *> staticEntities;
std::vector<AnimatedEntity *> entities;

double lastScreenX;
double lastScreenY;
int goalIndex = 0;
int numGoals = 4;
Eigen::Vector3f goals[4];
Eigen::Vector3f limbNormals[4];

bool keyToggles[512] = {false};
bool mouseToggle = false;

Model * guyModel, * jackModel;
StaticEntity * skyEnt, * terrainEnt, * jackEnt;
MocapEntity * chebEnt;
SkinnedEntity * trexEnt;
IKEntity * climberEnt;

StaticShader * staticShader;
AnimatedShader * animShader;
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
            terrainGenerator->UpdateMesh(camera->position, 1000);
            break;
         default:
            keyToggles[key] = true;
            break;
      }
   } else if (action == GLFW_RELEASE) {
      switch (key) {
         case GLFW_KEY_ENTER:
            goalIndex = (goalIndex + 1) % numGoals;
            break;
         case GLFW_KEY_M:
            guyModel->loadConstraints("assets/joints/guy.jnt");
            break;
         case GLFW_KEY_I:
            keyToggles[key] = !keyToggles[key];
            keyToggles[key] ?
               climberEnt->animateWithIK() :
               climberEnt->animateWithKeyframes();
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

         if (keyToggles[GLFW_KEY_I]) {
            Eigen::Vector2f ndc = calculateNDC(window);
            Geom::Rayf mouseRay(camera->position, camera->rayFromNDCToWorld(ndc(0), -ndc(1)));

            float leastDistSq = mouseRay.squaredDistToPoint(climberEnt->ikLimbs[0]->goal);
            int leastNdx = 0;
            for (int limbNdx = 1; limbNdx < 4; limbNdx++) {
               float distSq = mouseRay.squaredDistToPoint(climberEnt->ikLimbs[limbNdx]->goal);
               if (distSq < leastDistSq) {
                  leastDistSq = distSq;
                  leastNdx = limbNdx;
               }
            }
            goalIndex = leastNdx;
         }
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
   camera->setFOVY(fov);
}

// ======================================================================== //
// ========================== INITIALIZATION CODE ========================= //
// ======================================================================== //

static void setupLights() {
   lightData.lights[0].direction = Eigen::Vector3f(0.881, -0.365, 0.292);
   lightData.lights[0].color = 1.2 * Eigen::Vector3f(0.7, 0.44, 0.38);

   lightData.lights[1].direction = Eigen::Vector3f(-0.881, 0.365, -0.292);
   lightData.lights[1].color = 1.2 * Eigen::Vector3f(0.03, 0.05, 0.2);

   lightData.numLights = 2;
}

static void initialize() {
   staticShader = new StaticShader();
   animShader = new AnimatedShader();
   texShader = new TextureShader();
   camera = new Camera(Eigen::Vector3f(0,0,10));
   fov = 1.0;
   camera->setFOVY(fov);
   setupLights();

   // Skybox
   Model * skyModel = new Model();
   skyModel->loadCIAB("assets/models/skybox.ciab");
   skyModel->loadTexture("assets/textures/night_DIFF.png", false);
   skyEnt = new StaticEntity(Eigen::Vector3f(0,-250,0),
                            Eigen::Quaternionf(1,0,0,0),
                            Eigen::Vector3f(500,500,500),
                            skyModel);

   // Terrain Stuff
   terrainGenerator = new TerrainGenerator();
   Model * terrainModel = terrainGenerator->GenerateModel();
   terrainModel->loadTexture("assets/textures/rock_DIFF.png", true);
   terrainModel->loadNormalMap("assets/textures/rock_NORM.png", true);
   terrainModel->loadSpecularMap("assets/textures/rock_SPEC.png", true);
   terrainEnt = new StaticEntity(Eigen::Vector3f(0, 0, 0), terrainModel);

   // Animated Entities
   Model * chebModel = new Model();
   chebModel->loadOBJ("assets/cheb/cheb2.obj");
   chebModel->loadSkinningPIN("assets/cheb/cheb_attachment.txt");
   chebModel->loadAnimationPIN("assets/cheb/cheb_skel_walkAndSkip.txt");
   chebEnt = new MocapEntity(Eigen::Vector3f(-10, 0, 20), chebModel);
   chebEnt->playAnimation(0);

   Model * trexModel = new Model();
   trexModel->loadCIAB("assets/models/trex.ciab");
   trexModel->loadTexture("assets/textures/masonry_DIFF.png", false);
   trexModel->loadNormalMap("assets/textures/masonry_NORM.png", false);
   trexEnt = new SkinnedEntity(Eigen::Vector3f(10, 0, 20), trexModel);
   trexEnt->playAnimation(0);

   // Lumberjack
   jackModel = new Model();
   jackModel->loadCIAB("assets/models/lumberJack.ciab");
   jackModel->loadTexture("assets/textures/lumberJack_DIFF.png", true);
   jackModel->loadNormalMap("assets/textures/lumberJack_NORM.png", true);
   jackEnt = new SkinnedEntity(Eigen::Vector3f(0, 0, 20), jackModel);

   // The main character
   guyModel = new Model();
   guyModel->loadCIAB("assets/models/guy.ciab");
   guyModel->loadTexture("assets/textures/guy_DIFF.bmp", false);
   guyModel->loadConstraints("assets/joints/guy.jnt");
   climberEnt = new IKEntity(Eigen::Vector3f(0, 0, 5), guyModel);
   climberEnt->playAnimation(0);

   // Set up limbs
   std::vector<int> boneIndices = std::vector<int>(0);
   boneIndices.push_back(0);
   boneIndices.push_back(1);
   boneIndices.push_back(2);
   boneIndices.push_back(3);
   boneIndices.push_back(9);
   boneIndices.push_back(10);
   boneIndices.push_back(11);
   boneIndices.push_back(12);
   boneIndices.push_back(13);
   climberEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), true);
   boneIndices.clear();

   boneIndices.push_back(0);
   boneIndices.push_back(1);
   boneIndices.push_back(2);
   boneIndices.push_back(3);
   boneIndices.push_back(15);
   boneIndices.push_back(16);
   boneIndices.push_back(17);
   boneIndices.push_back(18);
   boneIndices.push_back(19);
   climberEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), true);
   boneIndices.clear();

   boneIndices.push_back(0);
   boneIndices.push_back(21);
   boneIndices.push_back(22);
   boneIndices.push_back(23);
   boneIndices.push_back(24);
   boneIndices.push_back(25);
   climberEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), true);
   boneIndices.clear();

   boneIndices.push_back(0);
   boneIndices.push_back(26);
   boneIndices.push_back(27);
   boneIndices.push_back(28);
   boneIndices.push_back(29);
   boneIndices.push_back(30);
   climberEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), true);
   boneIndices.clear();
}

// ======================================================================== //
// ============================== UPDATE CODE ============================= //
// ======================================================================== //

static void updateCamera(GLFWwindow * window, double timePassed) {

   if (keyToggles[GLFW_KEY_T]) {
      Eigen::Vector2f ndc = calculateNDC(window);
      Eigen::Vector3f mouseOffset = camera->rayFromNDCToView(-ndc(0), -ndc(1));

      Eigen::Quaternionf mouseRotation = Eigen::Quaternionf::FromTwoVectors(FORWARD_BASE, mouseOffset);
      Eigen::Quaternionf destQuat = climberEnt->rotation * mouseRotation;
      camera->smoothFollow(climberEnt->position - 20 * climberEnt->getForward() + 15 * climberEnt->getUp(), destQuat);
   } else {
      float distTraveled = 20 * timePassed;

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

   if (keyToggles[GLFW_KEY_Q])
      camera->aim(0,0, -0.05);
   if (keyToggles[GLFW_KEY_E])
      camera->aim(0,0, 0.05);

   // camGoal = camera->position + 10 * camera->getForward();
}

double timeCount = 0;

static void updateEntities(GLFWwindow * window, double timePassed) {
   skyEnt->position = camera->position + Eigen::Vector3f(0, -250, 0);

   timeCount += timePassed;
   if (timeCount > 0 && timeCount < 2) {
      terrainGenerator->UpdateMesh(climberEnt->position + Eigen::Vector3f(0,5,0), 20);
      timeCount -= 0.05;
   }

   if (keyToggles[GLFW_KEY_I] && mouseToggle) {
      Eigen::Vector2f ndc = calculateNDC(window);
      Geom::Rayf mouseRay(camera->position, camera->rayFromNDCToWorld(ndc(0), -ndc(1)));

      for (int i = 0; i < terrainGenerator->model->faces.size(); i++) {
         Face * f = terrainGenerator->model->faces[i];
         Eigen::Vector3f pnt = f->intersectRay(mouseRay);

         if (f->pointCheckInside(pnt)) {
            climberEnt->setLimbGoal(goalIndex, pnt);
            camGoal = pnt;
            break;
         }
      }
   }

   if (keyToggles[GLFW_KEY_T]) {
      float distTraveled = 20 * timePassed;
      float radiansTraveled = 5 * timePassed;

      if (keyToggles[GLFW_KEY_W])
         climberEnt->moveForward(distTraveled);
      if (keyToggles[GLFW_KEY_S])
         climberEnt->moveBackward(distTraveled);
      if (keyToggles[GLFW_KEY_A])
         climberEnt->rotateAlong(radiansTraveled, UP_BASE);
      if (keyToggles[GLFW_KEY_D])
         climberEnt->rotateAlong(-radiansTraveled, UP_BASE);
      if (keyToggles[GLFW_KEY_SPACE])
         climberEnt->moveUp(distTraveled);
      if (keyToggles[GLFW_KEY_LEFT_SHIFT])
         climberEnt->moveDown(distTraveled);
   }

   chebEnt->update(timePassed);
   trexEnt->update(timePassed);
   climberEnt->update(timePassed);

   // chebEnt->position += Eigen::Vector3f(0.01, 0, 0);
}

static void draw(double deltaTime) {
   texShader->render(camera, & lightData, skyEnt);

   staticShader->render(camera, & lightData, terrainEnt);
   staticShader->render(camera, & lightData, jackEnt);

   animShader->render(camera, & lightData, chebEnt);
   animShader->render(camera, & lightData, trexEnt);
   animShader->render(camera, & lightData, climberEnt);

   if (keyToggles[GLFW_KEY_K]) {
      animShader->renderVertices(camera, terrainEnt);
      animShader->renderBones(camera, climberEnt);
      animShader->renderPaths(camera, terrainGenerator);
   }

   animShader->renderPoint(camera, climberEnt->ikLimbs[goalIndex]->goal);
   animShader->renderPoint(camera, camGoal);
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
