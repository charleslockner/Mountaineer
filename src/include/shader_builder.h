#ifndef __SHADER_BUILDER_H__
#define __SHADER_BUILDER_H__

/* Builds a vertex and fragment shader from the two given
	paths to glsl files. Returns an integer representing
	the opengl shader program handle */
namespace SB {
   unsigned int BuildProgramFromPaths(const char * vertPath, const char * fragPath);
   unsigned int BuildProgramFromStrings(const char * vertString, const char * fragString);
}

#endif
