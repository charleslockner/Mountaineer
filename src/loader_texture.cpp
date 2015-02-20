
// FOR THE MIP MAP CALL THAT IS NOT PRESENT IN GLFW
#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "safe_gl.h"
#include "model.h"

void Model::loadTexture(const char * filename) {

   // Load texture
   int w, h, ncomps;
   unsigned char *data = stbi_load(filename, &w, &h, &ncomps, 0);

   if(!data) {
      std::cerr << filename << " not found" << std::endl;
      exit(1);
   }
   if(ncomps != 3) {
      std::cerr << filename << " must have 3 components (RGB)" << std::endl;
      exit(1);
   }
   if((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
      std::cerr << filename << " must be a power of 2" << std::endl;
      exit(1);
   }

   // Generate a texture buffer object
   glGenTextures(1, & this->texID);
   // Bind the current texture to be the newly generated texture object
   glBindTexture(GL_TEXTURE_2D, this->texID);
   // Load the actual texture data
   // Base level is 0, number of channels is 3, and border is 0.
   glTexImage2D(GL_TEXTURE_2D, 0, ncomps, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
   // Generate image pyramid
   glGenerateMipmap(GL_TEXTURE_2D);
   // Set texture wrap modes for the S and T directions
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   // Set filtering mode for magnification and minimification
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   // Unbind
   glBindTexture(GL_TEXTURE_2D, 0);
   // Free image, since the data is now on the GPU
   stbi_image_free(data);

   this->hasTextures = true;
   std::cerr << "Loaded texture: " << filename << "\n";
}
