#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <OpenGL/gl.h>

#include "shader_builder.h"

#define printOpenGLError() printOglError(__FILE__, __LINE__)

static int printOglError (const char *file, int line) {
   /* Returns 1 if an OpenGL error occurred, 0 otherwise. */
   GLenum glErr;
   int    retCode = 0;

   glErr = glGetError ();
   while (glErr != GL_NO_ERROR) {
      printf ("glError in file %s @ line %d.\n", file, line);
      retCode = 1;
      glErr = glGetError ();
   }
   return retCode;
}

static void printShaderInfoLog (GLuint shader)
{
   GLint     infologLength = 0;
   GLint     charsWritten  = 0;
   GLchar *infoLog;

   printOpenGLError ();  // Check for OpenGL errors
   glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infologLength);
   printOpenGLError ();  // Check for OpenGL errors

   if (infologLength > 0) {
      infoLog = (GLchar *)malloc (infologLength);
      if (infoLog == NULL) {
         puts ("ERROR: Could not allocate InfoLog buffer");
         exit (1);
         }
      glGetShaderInfoLog (shader, infologLength, &charsWritten, infoLog);
      printf ("Shader InfoLog:\n%s\n\n", infoLog);
      free (infoLog);
      }
   printOpenGLError();  // Check for OpenGL errors
}

/* Print out the information log for a program object */
static void printProgramInfoLog (GLuint program)
{
   GLint     infologLength = 0;
   GLint     charsWritten  = 0;
   GLchar *infoLog;

   printOpenGLError ();  // Check for OpenGL errors
   glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infologLength);
   printOpenGLError ();  // Check for OpenGL errors

   if (infologLength > 0)
      {
      infoLog = (GLchar *)malloc (infologLength);
      if (infoLog == NULL)
         {
         puts ("ERROR: Could not allocate InfoLog buffer");
         exit (1);
         }
      glGetProgramInfoLog (program, infologLength, &charsWritten, infoLog);
      printf ("Program InfoLog:\n%s\n\n", infoLog);
      free (infoLog);
      }
   printOpenGLError ();  // Check for OpenGL errors
}

static void getGLversion() {
   int major, minor;

   major = minor =0;
   const char *verstr = (const char *)glGetString(GL_VERSION);

   if ((verstr == NULL) || (sscanf(verstr, "%d.%d", &major, &minor) !=2)) {
      printf("Invalid GL_VERSION format %d %d\n", major, minor);
   }
   if( major <2) {
      printf("This shader will not work due to opengl version, which is %d %d\n", major, minor);
      exit(0);
   }
}

static char * textFileRead(const char * path) {
   FILE *fp;
   char *content = NULL;

   int count=0;

   if (path) {
      fp = fopen(path,"rt");
      if (fp) {

         fseek(fp, 0, SEEK_END);
         count = ftell(fp);
         rewind(fp);

         if (count > 0) {
            content = (char *)malloc(sizeof(char) * (count+1));
            count = fread(content,sizeof(char),count,fp);
            content[count] = '\0';
         }
         fclose(fp);
      } else {
         printf("Error loading %s\n: ", path);
         perror("");
         exit(1);
      }
   } else {
      fprintf(stderr, "Error reading shader file. Filename string empty\n");
      exit(1);
   }

   return content;
}

static GLuint buildShaderFromString(const char * srcCode, GLuint type) {
   GLint compileStatus;
   GLuint handle = glCreateShader(type);

   glShaderSource(handle, 1, &srcCode, NULL);

   glCompileShader(handle);
   printOpenGLError();
   glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
   printShaderInfoLog(handle);

   if (!compileStatus) {
      printf("Error compiling shader: %s\n", srcCode);
      exit(1);
   }

   return handle;
}

static GLuint buildShaderFromPath(const char * srcPath, GLuint type) {
   char * src = textFileRead(srcPath);
   int shaderHandle = buildShaderFromString(src, type);
   free(src);

   return shaderHandle;
}

static GLuint buildProgram(GLuint vs, GLuint fs) {
   GLint linked;
   GLuint program = glCreateProgram();

   if (!program) {
      printf("Error compiling shader program\n");
      exit(1);
   }

   glAttachShader(program, vs);
   glAttachShader(program, fs);
   glLinkProgram(program);

   printOpenGLError();
   glGetProgramiv(program, GL_LINK_STATUS, &linked);
   printProgramInfoLog(program);
   getGLversion();

   printf("Sucessfully installed shader %d\n", program);

   return program;
}

GLuint SB_buildFromPaths(const char * vertPath, const char * fragPath) {
   GLuint vs = buildShaderFromPath(vertPath, GL_VERTEX_SHADER);
   GLuint fs = buildShaderFromPath(fragPath, GL_FRAGMENT_SHADER);

   return buildProgram(vs, fs);
}

GLuint SB_buildFromSrings(const char * vertString, const char * fragString) {
   GLuint vs = buildShaderFromString(vertString, GL_VERTEX_SHADER);
   GLuint fs = buildShaderFromString(fragString, GL_FRAGMENT_SHADER);

   return buildProgram(vs, fs);
}
