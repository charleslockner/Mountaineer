#ifndef __SHADER_BUILDER_H__
#define __SHADER_BUILDER_H__

/* Builds a vertex and fragment shader from the two given
	paths to glsl files. Returns an integer representing
	the opengl shader program handle */
unsigned int SB_buildFromPaths(const char * vertPath, const char * fragPath);
unsigned int SB_buildFromSrings(const char * vertString, const char * fragString);

#endif
