#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "mathc.h"
#include "model.h"

void drawMesh(Mesh *mesh, unsigned int shaderID, GLenum mode,
              mfloat_t *position, mfloat_t *rotation, mfloat_t scale);
void drawInstanced(Mesh *mesh, unsigned int shaderID, GLenum mode, int num,
                   mfloat_t scale);

#endif
