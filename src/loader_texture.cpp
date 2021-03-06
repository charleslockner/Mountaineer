
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "safe_gl.h"
#include "model.h"

static unsigned int loadImage(const char * filename, bool repeat) {
   unsigned int id;

   // Load texture
   int w, h, ncomps;
   unsigned char *stbi_data = stbi_load(filename, &w, &h, &ncomps, 0);

   if(!stbi_data) {
      std::cerr << filename << " not found\n";
      exit(1);
   }
   if(ncomps != 3) {
      std::cerr << filename << " must have 3 components (RGB)\n";
      exit(1);
   }
   if((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
      std::cerr << filename << " must be a power of 2\n";
      exit(1);
   }

   // Flip the image for opengl
   int rowSize = ncomps * w;
   unsigned char * data = (unsigned char *)malloc(sizeof(unsigned char) * rowSize * h);
   for (int i = 0; i < h; i++)
      memcpy(& data[rowSize * i], & stbi_data[rowSize * (h-1-i)], rowSize);

   // Free the stbi buffer because now we're using "data"
   stbi_image_free(stbi_data);
   // Generate a texture buffer object
   glGenTextures(1, & id);
   // Bind the current texture to be the newly generated texture object
   glBindTexture(GL_TEXTURE_2D, id);
   // Load the actual texture data
   // Base level is 0, number of channels is 3, and border is 0.
   glTexImage2D(GL_TEXTURE_2D, 0, ncomps, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
   // Generate image pyramid
   glGenerateMipmap(GL_TEXTURE_2D);
   // Set texture wrap modes for the S and T directions
   int glRepeatConst = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glRepeatConst);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glRepeatConst);
   // Set filtering mode for magnification and minimification
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

   // Unbind
   glBindTexture(GL_TEXTURE_2D, 0);
   // Free the image data, since the data is now on the GPU
   free(data);

   return id;
}

void Model::loadTexture(const char * filename, bool repeat) {
   this->texID = loadImage(filename, repeat);
   this->hasTexture = true;
   std::cerr << "Loaded texture: " << filename << "\n";
}

void Model::loadNormalMap(const char * filename, bool repeat) {
   this->nmapID = loadImage(filename, repeat);
   this->hasNormalMap = true;
   std::cerr << "Loaded normal map: " << filename << "\n";
}

void Model::loadSpecularMap(const char * filename, bool repeat) {
   this->smapID = loadImage(filename, repeat);
   this->hasSpecularMap = true;
   std::cerr << "Loaded specular map: " << filename << "\n";
}


