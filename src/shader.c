#include <stdio.h>
#include <stdlib.h>

#include "dependencies/include/GL/glew.h"

#include "shader.h"
#include "util.h"

unsigned createShader(const char *vertexFile, const char *fragmentFile) {
  GLint success = 0;
  GLint logSize = 0;

  // Create a shader object and compile it during runtime
  const unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertexShaderSource = readFile(vertexFile);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar infoLog[logSize];
    glGetShaderInfoLog(vertexShader, logSize, &logSize, infoLog);
    glDeleteShader(vertexShader);
    printf("Vertex Shader: %s\n", infoLog);
  }

  // Perform the same steps for the fragment shader
  const unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragmentShaderSource = readFile(fragmentFile);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar infoLog[logSize];
    glGetShaderInfoLog(fragmentShader, logSize, &logSize, infoLog);
    glDeleteShader(fragmentShader);
    printf("Fragment Shader: %s\n", infoLog);
  }

  // Create a shader program and link the two shader steps together
  const unsigned shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Make sure to cleanup the individual shaders after linking
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Return the program
  return shaderProgram;
}

void detachShader() { glUseProgram(0); } // never used

// never used
void destroyShader(unsigned shaderID) { glDeleteProgram(shaderID); }
