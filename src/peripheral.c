#include "peripheral.h"

#include <stdlib.h>

Mouse *createMouse() { return calloc(1, sizeof(Mouse)); }

void updateMouse(GLFWwindow *window, Mouse *mouse) {
  mouse->pxpos = mouse->xpos;
  mouse->pypos = mouse->ypos;
  glfwGetCursorPos(window, &(mouse->xpos), &(mouse->ypos));
}

double getDx(Mouse *mouse) { return mouse->xpos - mouse->pxpos; }
double getDy(Mouse *mouse) { return mouse->ypos - mouse->pypos; }
