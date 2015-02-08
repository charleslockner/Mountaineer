#ifndef __TEXTURE_LOADER__
#define __TEXTURE_LOADER__

typedef struct Image {
   unsigned long sizeX;
   unsigned long sizeY;
   char *data;
} Image;

Image * Tl_createTexture(const char * path);

#endif
