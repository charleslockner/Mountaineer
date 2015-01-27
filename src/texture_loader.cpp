#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>

#include "model_builder.h"

/* getint and getshort are help functions to load the bitmap byte by byte */
static unsigned int getint(FILE *fp) {
   int c, c1, c2, c3;

   /*  get 4 bytes */
   c = getc(fp);
   c1 = getc(fp);
   c2 = getc(fp);
   c3 = getc(fp);

   return ((unsigned int) c) +
    (((unsigned int) c1) << 8) +
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
   int c, c1;

   /* get 2 bytes*/
   c = getc(fp);
   c1 = getc(fp);

   return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap (BMP) loader...for 24 bit bitmaps with 1 plane only.  */
static int createTBO(FILE *fp) {
   unsigned long image_size;           /*  size of the image in bytes. */
   unsigned short int planes;          /*  number of planes in image (must be 1)  */
   unsigned short int bpp;             /*  number of bits per pixel (must be 24) */

   // seek through the bmp header, up to the width height
   fseek(fp, 18, SEEK_CUR);

   unsigned long sizeX = getint(fp);
   unsigned long sizeY = getint(fp);
   image_size = sizeX * sizeY * 3;

   /*  read the planes */
   planes = getshort(fp);
   if (planes != 1) {
      printf("Plane count is %u, not 1.\n", planes);
      return -1;
   }

   /*  read the bpp */
   bpp = getshort(fp);
   if (bpp != 24) {
      printf("Bpp from is %u, not 24.\n", bpp);
      return -1;
   }

   // seek past the rest of the bitmap header
   fseek(fp, 24, SEEK_CUR);

   // read the data.
   char * image_data = (char *) malloc(image_size);
   if (image_data == NULL) {
      printf("Error allocating memory for color-corrected image data.\n");
      return -1;
   }

   if (fread(image_data, image_size, 1, fp) != 1) {
      printf("Error reading image data.\n");
      return -1;
   }

   unsigned int tbo;
   glGenTextures(1, & tbo);
   glBindTexture(GL_TEXTURE_2D, tbo);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                image_data);

   free(image_data);

   return tbo;
}

unsigned int MB_loadTexture(const char * path) {
   FILE * fp = safe_fopen(path);

   int tbo = createTBO(fp);
   if (tbo < 0) {
      printf("Cannot create texture from %s\n", path);
      exit(1);
   }

   fclose(fp);

   return tbo;
}
