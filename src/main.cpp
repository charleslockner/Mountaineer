#include <stdio.h>
#include <stdlib.h>

#include "safe_gl.h"
#include "shader.h"
#include "light.h"
#include "model.h"
#include "entity.h"
#include "terrain.h"
#include "grid.h"

#include <vector>

#define WIN_HEIGHT 600
#define WIN_WIDTH  1000

#define CAMERA_SPEED 5.0

LightData lightData;
Camera * camera;
std::vector<StaticEntity *> staticEntities;
std::vector<AnimatedEntity *> entities;

double lastScreenX;
double lastScreenY;
int goalIndex = 0;
Eigen::Vector3f goals[2];
bool playing = false;

bool keyToggles[512] = {false};

Model * guyModel;
IKEntity * guyEnt;
StaticEntity * skyEnt;
SpatialGrid * grid;
EntityShader * entShader;
TextureShader * texShader;

Eigen::Vector3f targetPoint;

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
            if (playing)
               guyEnt->playAnimation(0, 0, true);
            else
               guyEnt->stopAnimation(0, true);
            break;
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
   lightData.lights[0].direction = Eigen::Vector3f(-1.0, -0.5, -0.3).normalized();
   lightData.lights[0].color = Eigen::Vector3f(0.8, 0.7, 0.6);
   lightData.lights[0].strength = 250;
   lightData.lights[0].attenuation = 50.0;
   lightData.lights[0].spread = 15;

   lightData.lights[1].position = Eigen::Vector3f(18.0, 10.0, 10.0);
   lightData.lights[1].direction = Eigen::Vector3f(1.0, 0.5, 0.3).normalized();
   lightData.lights[1].color = Eigen::Vector3f(0.15, 0.12, 0.27);
   lightData.lights[1].strength = 250;
   lightData.lights[1].attenuation = 50.0;
   lightData.lights[1].spread = 15;

   lightData.numLights = 2;
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
   glClearColor(0.2,0.2,0.2,1);

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
   skyEnt->position = camera->position + Eigen::Vector3f(0, -250, 0);

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

   Eigen::Vector3f camGoal = camera->position + 10 * camera->direction.normalized();
   Vertex * vert = grid->FindClosest(camGoal, 10);
   if (vert)
      guyEnt->setLimbGoal(0, vert->position);
   vert = grid->FindClosest(camGoal + Eigen::Vector3f(0, -5, 0), 10);
   if (vert)
      guyEnt->setLimbGoal(1, vert->position);
}

void initialize() {
   entShader = new ForwardShader();
   texShader = new TextureShader();
   camera = new Camera(Eigen::Vector3f(0,0,10), Eigen::Vector3f(0,0,-1), Eigen::Vector3f(0,1,0));
   setupLights();

   // Skybox
   Model * skyModel = new Model();
   skyModel->loadCIAB("assets/models/skybox.ciab");
   skyModel->loadTexture("assets/textures/night.png", false);
   skyEnt = new StaticEntity(Eigen::Vector3f(0,-250,0),
                             Eigen::Quaternionf(1,0,0,0),
                             Eigen::Vector3f(500,500,500),
                             skyModel);
   staticEntities.push_back(skyEnt);

   // Terrain Stuff
   TerrainGenerator * terrainGenerator = new TerrainGenerator();
   Model * terrainModel = terrainGenerator->generateRockFace();
   terrainModel->loadTexture("assets/textures/rock.png", true);
   terrainModel->loadNormalMap("assets/textures/rock_NORM.png", true);
   terrainModel->loadSpecularMap("assets/textures/rock_SPEC.png", true);
   staticEntities.push_back(new StaticEntity(Eigen::Vector3f(0, 0, 0), terrainModel));

   grid = new SpatialGrid(1000, 20.0);
   for (int i = 0; i < terrainModel->vertices.size(); i++)
      grid->Add(& terrainModel->vertices[i]);

   // Animated Entities
   Model * chebModel = new Model();
   chebModel->loadOBJ("assets/cheb/cheb2.obj");
   chebModel->loadSkinningPIN("assets/cheb/cheb_attachment.txt");
   chebModel->loadAnimationPIN("assets/cheb/cheb_skel_walkAndSkip.txt");
   entities.push_back(new MocapEntity(Eigen::Vector3f(-10, 0, 0), chebModel));
   entities[0]->playAnimation(0);

   Model * trexModel = new Model();
   trexModel->loadCIAB("assets/models/trex.ciab");
   trexModel->loadTexture("assets/textures/masonry.png", false);
   trexModel->loadNormalMap("assets/textures/masonry_NORM.png", false);
   entities.push_back(new BonifiedEntity(Eigen::Vector3f(10, 0, 0), trexModel));
   entities[1]->playAnimation(0);
   trexModel->bufferIndices();

   // The main character
   guyModel = new Model();
   guyModel->loadCIAB("assets/models/guy.ciab");
   guyModel->loadTexture("assets/textures/guy_tex.bmp", false);
   guyModel->loadConstraints("assets/models/guy.cns");
   guyEnt = new IKEntity(Eigen::Vector3f(0, 0, 0), guyModel);
   trexModel->bufferIndices();

   std::vector<int> boneIndices = std::vector<int>(0);
   boneIndices.push_back(0);
   boneIndices.push_back(26);
   boneIndices.push_back(27);
   boneIndices.push_back(28);
   boneIndices.push_back(29);
   boneIndices.push_back(30);
   guyEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), true);
   boneIndices.clear();
   boneIndices.push_back(0);
   boneIndices.push_back(1);
   boneIndices.push_back(2);
   boneIndices.push_back(3);
   boneIndices.push_back(9);
   boneIndices.push_back(10);
   boneIndices.push_back(11);
   boneIndices.push_back(12);
   boneIndices.push_back(13);
   guyEnt->addLimb(boneIndices, Eigen::Vector3f(0, 0, 0), GL_TRUE);

   entities.push_back(guyEnt);
   guyEnt->playAnimation(0);
}

void updateLoop(double deltaTime) {
   updateCameraPosition(deltaTime);
   updateWorld(deltaTime);

   texShader->render(camera, & lightData, skyEnt);
   entShader->render(camera, & lightData, staticEntities[1]);

   for (int i = 0; i < entities.size(); i++)
      entities[i]->update(deltaTime);
   for (int i = 0; i < entities.size(); i++)
      entShader->render(camera, & lightData, entities[i]);

   if (keyToggles[GLFW_KEY_K]) {
      entShader->renderVertices(camera, staticEntities[1]);
      entShader->renderVertices(camera, entities[1]);
      entShader->renderBones(camera, (BonifiedEntity *)(entities[1]));
      entShader->renderBones(camera, guyEnt);
   }

   entShader->renderPoint(camera, goals[goalIndex]);
   entShader->renderPoint(camera, targetPoint);
}

int main(int argc, char ** argv) {
   GLFWwindow * window = windowSetup();

   initialize(); // game code

   double timePassed = 0;
   unsigned int numFrames = 0;
   double lastTime = glfwGetTime();

   while (!glfwWindowShouldClose(window)) {
      timePassed = glfwGetTime();
      double deltaTime = timePassed - lastTime;
      lastTime = timePassed;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      updateLoop(deltaTime); // game code

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
