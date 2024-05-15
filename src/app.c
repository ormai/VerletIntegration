#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dependencies/include/GL/glew.h"
#include "dependencies/include/GLFW/glfw3.h"

#include "camera.h"
#include "graphics.h"
#include "model.h"
#include "peripheral.h"
#include "shader.h"
#include "verlet.h"

// Preprocessor constants
#define ANIMATION_TIME 90.0f // Frames
#define ADDITION_SPEED 10
#define TARGET_FPS 60
#define NUM_SUBSTEPS 8

struct {
  bool cursorEntered;
  Camera *camera;
  float cameraRadius;
  int totalFrames;
} state = {false, NULL, 24, 0};

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly

static void updateCamera(GLFWwindow *window, Mouse *mouse, Camera *camera) {
  const float speed = 0.08f;
  mfloat_t temp[VEC3_SIZE];

  const float universalAngle = state.totalFrames / 4.;
  vec3(camera->position, MCOS(MRADIANS(universalAngle)) * state.cameraRadius,
       camera->position[1],
       MSIN(MRADIANS(universalAngle)) * state.cameraRadius);
  camera->yaw = universalAngle + 180.;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    vec3_add(camera->position, camera->position,
             vec3_multiply_f(temp, camera->up, speed));
    camera->pitch -= .22;
    state.cameraRadius -= .01;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    vec3_subtract(camera->position, camera->position,
                  vec3_multiply_f(temp, camera->up, speed));
    camera->pitch += .22;
    state.cameraRadius += .01;
  }

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    vec3_add(camera->position, camera->position,
             vec3_multiply_f(temp, camera->up, speed));

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    vec3_subtract(camera->position, camera->position,
                  vec3_multiply_f(temp, camera->up, speed));

  // // Mouse Movement
  if (state.cursorEntered) {
    double dx = .1 * getDx(mouse);
    double dy = .1 * getDy(mouse);
    camera->yaw += dx;

    camera->pitch -= dy;
    if (camera->pitch > 89.)
      camera->pitch = 89.;
    if (camera->pitch < -89.)
      camera->pitch = -89.;
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

static void cursor_enter_callback(GLFWwindow *window, int entered) {
  if (entered) {
    // The cursor entered the content area of the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.cursorEntered = true;
  } else {
    // The cursor left the content area of the window
  }
}

static void instantiateVerlets(VerletObject *objects, int size) {
  const float distance = 7;
  for (int i = 0; i < size; i++) {

    // ====== LOOP ======
    float x = MSIN(i) * distance;
    float z = MCOS(i) * distance;
    float xp = MSIN(i) * distance * .999;
    float zp = MCOS(i) * distance * .999;
    float y = rand() % (2 - 1 + 1) + 1;

    VerletObject *obj = &objects[i];
    vec3(obj->current, x, y, z);
    vec3(obj->previous, xp, y, zp);
    vec3(obj->acceleration, 0, 0, 0);
    obj->radius = VERLET_RADIUS;

    // ====== STREAM ======
    /*float x = (0 + i) % (int)distance - distance / 2;*/
    /*float y = -2.0f;*/
    /*float z = -4.0f;*/
    /*float xp = x * 1.005;*/
    /*float yp = y * 1.002;*/
    /*float zp = z * 1.005;*/
    /*vec3(obj->current, x, y, z);*/
    /*vec3(obj->previous, xp, yp, zp);*/
    /*vec3(obj->acceleration, 0, 0, 0);*/
    /*obj->radius = VERLET_RADIUS;*/
  }
}

int main(void) {
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  /* Create a windowed mode window and its OpenGL context */
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Verlet Integration", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  /* Set up a callback function for when the window is resized */
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSwapInterval(1);

  /* Callback function for mouse enter/exit */
  glfwSetCursorEnterCallback(window, cursor_enter_callback);

  /* Initialize GLEW */
  glewInit();

  /* OpenGL Settings */
  glClearColor(.1, .1, .1, 1.);
  glClearStencil(0);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPointSize(3.);

  /* Models & Shaders */
  unsigned phongShader =
      createShader("shaders/phong_vertex.glsl", "shaders/phong_fragment.glsl");
  unsigned instanceShader = createShader("shaders/instance_vertex.glsl",
                                         "shaders/instance_fragment.glsl");
  unsigned baseShader =
      createShader("shaders/base_vertex.glsl", "shaders/base_fragment.glsl");

  Mesh *mesh = createMesh("models/sphere.obj", true);
  Mesh *cubeMesh = createMesh("models/cube.obj", false);

  // Model* model = createModel(mesh);

  // Container
  mfloat_t containerPosition[VEC3_SIZE] = {0, 0, 0};
  mfloat_t rotation[VEC3_SIZE] = {0, 0, 0};

  VerletObject *verlets = malloc(sizeof(VerletObject) * MAX_INSTANCES);
  instantiateVerlets(verlets, MAX_INSTANCES);
  int numActive = 0;

  mfloat_t view[MAT4_SIZE];
  state.camera = createCamera((mfloat_t[]){0, 0, state.cameraRadius});

  Mouse *mouse = createMouse();

  float dt = .000001;
  float lastFrameTime = (float)glfwGetTime();

  char title[100] = "";

  srand(time(NULL));

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Input */
    updateMouse(window, mouse);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // extert center force
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
      addForce(verlets, numActive, (mfloat_t[]){0, 3, 0},
               -30. * NUM_SUBSTEPS);

    /* Camera */
    updateCamera(window, mouse, state.camera);
    createViewMatrix(view, state.camera);

    /* Shader Uniforms */
    glUseProgram(phongShader);
    glUniformMatrix4fv(glGetUniformLocation(phongShader, "view"), 1, GL_FALSE,
                       view);
    glUseProgram(0);
    glUseProgram(baseShader);
    glUniformMatrix4fv(glGetUniformLocation(baseShader, "view"), 1, GL_FALSE,
                       view);
    glUseProgram(0);
    glUseProgram(instanceShader);
    glUniformMatrix4fv(glGetUniformLocation(instanceShader, "view"), 1,
                       GL_FALSE, view);
    glUseProgram(0);

    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (1.0 / dt >= TARGET_FPS - 5 &&
        glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS &&
        numActive < MAX_INSTANCES) {
      numActive += ADDITION_SPEED;
    }

    if (state.totalFrames % 60 == 0) {
      sprintf(title, "FPS : %-4.0f | Balls : %-10d", 1. / dt, numActive);
      glfwSetWindowTitle(window, title);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      containerPosition[0] -= .05;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      containerPosition[0] += .05;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
      containerPosition[1] -= .05;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
      containerPosition[1] += .05;

    float sub_dt = dt / NUM_SUBSTEPS;
    for (int i = 0; i < NUM_SUBSTEPS; i++) {
      applyForces(verlets, numActive);
      // applyCollisions(verlets, numActive);
      applyGridCollisions(verlets, numActive);
      applyConstraints(verlets, numActive, containerPosition);
      updatePositions(verlets, numActive, sub_dt);
    }

    float verletPositions[numActive * VEC3_SIZE];
    float verletVelocities[numActive];

    int posPointer = 0;
    int velPointer = 0;
    for (int i = 0; i < numActive; i++) {
      VerletObject verlet = verlets[i];
      verletPositions[posPointer++] = verlet.current[0];
      verletPositions[posPointer++] = verlet.current[1];
      verletPositions[posPointer++] = verlet.current[2];
      float vel = vec3_distance(verlet.current, verlet.previous) * 10;
      verletVelocities[velPointer++] = vel;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->positionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(float) * INSTANCE_STRIDE * numActive,
                    verletPositions);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->velocityVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numActive,
                    verletVelocities);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Draw instanced verlet objects */
    drawInstanced(mesh, instanceShader, GL_TRIANGLES, numActive,
                  verlets[0].radius);

    /* Container */
    drawMesh(cubeMesh, baseShader, GL_TRIANGLES, containerPosition, rotation,
             CONTAINER_RADIUS * 2 + VERLET_RADIUS * 3);
    // drawMesh(mesh, baseShader, GL_TRIANGLES, containerPosition, rotation,
    // CONTAINER_RADIUS * 1.02); drawMesh(mesh, baseShader, GL_POINTS,
    // containerPosition, rotation, CONTAINER_RADIUS * 1.02);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();

    /* Timing */
    dt = (float)glfwGetTime() - lastFrameTime;
    while (dt < 1. / TARGET_FPS) {
      dt = (float)glfwGetTime() - lastFrameTime;
    }
    lastFrameTime = (float)glfwGetTime();
    state.totalFrames++;
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
