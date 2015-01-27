#ifndef __MODEL_BUILDER__
#define __MODEL_BUILDER__

#include <stdio.h>
#include "model.h"

FILE * safe_fopen(const char * path);
unsigned int MB_loadTexture(const char * path);

Model * MB_build(const char * meshPath, const char * texPath);
void MB_destroy(Model * model);

#endif // __MODEL_LOADER__