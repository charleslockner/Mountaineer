#ifndef __SAFE_GL_H__
#define __SAFE_GL_H__

// #ifdef __APPLE__
// #include <GLUT/glut.h>
// #endif
// #ifdef __unix__
// #include <GL/glut.h>
// #endif

// FOR THE MIP MAP CALL THAT IS NOT PRESENT IN GLFW
#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif

#define GL_GLEXT_PROTOTYPES
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <stdio.h>

// #define USE_SAFE_GL
// #define USE_SMART_USE_PROGRAM

#define checkOpenGLError() checkOglError(__FILE__, __LINE__)

inline void printOglError(GLenum err) {
   printf("Error code %d: ", err);

   switch (err) {
      case GL_NO_ERROR:
         printf("GL_NO_ERROR\n");
         break;
      case GL_INVALID_ENUM:
         printf("GL_INVALID_ENUM\n");
         break;
      case GL_INVALID_VALUE:
         printf("GL_INVALID_VALUE\n");
         break;
      case GL_INVALID_OPERATION:
         printf("GL_INVALID_OPERATION\n");
         break;
      // case GL_INVALID_FRAMEBUFFER_OPERATION:
      //    printf("The framebuffer object is not complete.\n");
      //    break;
      case GL_OUT_OF_MEMORY:
         printf("GL_OUT_OF_MEMORY\n");
         break;
      case GL_STACK_UNDERFLOW:
         printf("GL_STACK_UNDERFLOW\n");
         break;
      case GL_STACK_OVERFLOW:
         printf("GL_STACK_OVERFLOW\n");
         break;
      default:
         printf("Unknown error\n");
   }
}

inline void checkOglError(const char * file, int line) {
   GLenum err = glGetError();
   if (err != GL_NO_ERROR) {
      printf("glError in file %s at line %d: ", file, line);
      printOglError(err);
   }
}

inline GLint safe_glGetAttribLocation(const GLuint program, const char varname[]) {
   GLint r = glGetAttribLocation(program, varname);
   if (r < 0)
      printf("WARN: %s does not exist. glAttrib calls will silently ignore it.\n", varname);
   return r;
}
inline GLint safe_glGetUniformLocation(const GLuint program, const char varname[]) {
   GLint r = glGetUniformLocation(program, varname);
   if (r < 0)
      printf("WARN: %s does not exist. glUniform calls will silently ignore it.\n", varname);
   return r;
}

inline void safe_glEnableVertexAttribArray(const GLint handle) {
   if (handle >= 0)
      glEnableVertexAttribArray(handle);
}
inline void safe_glDisableVertexAttribArray(const GLint handle) {
   if (handle >= 0)
      glDisableVertexAttribArray(handle);
}
inline void safe_glVertexAttribPointer(const GLint handle, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {
   if (handle >= 0)
      glVertexAttribPointer(handle, size, type, normalized, stride, pointer);
}

inline void safe_glUniform1f(const GLint handle, const GLfloat a) {
   if (handle >= 0)
      glUniform1f(handle, a);
}
inline void safe_glUniform2f(const GLint handle, const GLfloat a, const GLfloat b) {
   if (handle >= 0)
      glUniform2f(handle, a, b);
}
inline void safe_glUniform3f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c) {
   if (handle >= 0)
      glUniform3f(handle, a, b, c);
}
inline void safe_glUniform4f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c, const GLfloat d) {
   if (handle >= 0)
      glUniform4f(handle, a, b, c, d);
}

inline void safe_glUniform1i(const GLint handle, const GLint a) {
   if (handle >= 0)
      glUniform1i(handle, a);
}
inline void safe_glUniform2i(const GLint handle, const GLint a, const GLint b) {
   if (handle >= 0)
      glUniform2i(handle, a, b);
}
inline void safe_glUniform3i(const GLint handle, const GLint a, const GLint b, const GLint c) {
   if (handle >= 0)
      glUniform3i(handle, a, b, c);
}
inline void safe_glUniform4i(const GLint handle, const GLint a, const GLint b, const GLint c, const GLint d) {
   if (handle >= 0)
      glUniform4i(handle, a, b, c, d);
}

// inline void safe_glUniform1ui(const GLint handle, const GLuint a) {
//    if (handle >= 0)
//       glUniform1ui(handle, a);
// }
// inline void safe_glUniform2ui(const GLint handle, const GLuint a, const GLuint b) {
//    if (handle >= 0)
//       glUniform2ui(handle, a, b);
// }
// inline void safe_glUniform3ui(const GLint handle, const GLuint a, const GLuint b, const GLuint c) {
//    if (handle >= 0)
//       glUniform3ui(handle, a, b, c);
// }
// inline void safe_glUniform4ui(const GLint handle, const GLuint a, const GLuint b, const GLuint c, const GLuint d) {
//    if (handle >= 0)
//       glUniform4ui(handle, a, b, c, d);
// }

inline void safe_glUniform1fv(const GLint handle, const GLsizei count, const GLfloat *value) {
   if (handle >= 0)
      glUniform1fv(handle, count, value);
}
inline void safe_glUniform2fv(const GLint handle, const GLsizei count, const GLfloat *value) {
   if (handle >= 0)
      glUniform2fv(handle, count, value);
}
inline void safe_glUniform3fv(const GLint handle, const GLsizei count, const GLfloat *value) {
   if (handle >= 0)
      glUniform3fv(handle, count, value);
}
inline void safe_glUniform4fv(const GLint handle, const GLsizei count, const GLfloat *value) {
   if (handle >= 0)
      glUniform4fv(handle, count, value);
}

inline void safe_glUniform1iv(const GLint handle, const GLsizei count, const GLint *value) {
   if (handle >= 0)
      glUniform1iv(handle, count, value);
}
inline void safe_glUniform2iv(const GLint handle, const GLsizei count, const GLint *value) {
   if (handle >= 0)
      glUniform2iv(handle, count, value);
}
inline void safe_glUniform3iv(const GLint handle, const GLsizei count, const GLint *value) {
   if (handle >= 0)
      glUniform3iv(handle, count, value);
}
inline void safe_glUniform4iv(const GLint handle, const GLsizei count, const GLint *value) {
   if (handle >= 0)
      glUniform4iv(handle, count, value);
}

// inline void safe_glUniform1uiv(const GLint handle, const GLsizei count, const GLuint *value) {
//    if (handle >= 0)
//       glUniform1uiv(handle, count, value);
// }
// inline void safe_glUniform2uiv(const GLint handle, const GLsizei count, const GLuint *value) {
//    if (handle >= 0)
//       glUniform2uiv(handle, count, value);
// }
// inline void safe_glUniform3uiv(const GLint handle, const GLsizei count, const GLuint *value) {
//    if (handle >= 0)
//       glUniform3uiv(handle, count, value);
// }
// inline void safe_glUniform4uiv(const GLint handle, const GLsizei count, const GLuint *value) {
//    if (handle >= 0)
//       glUniform4uiv(handle, count, value);
// }

inline void safe_glUniformMatrix2fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix2fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix3fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix3fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix4fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix4fv(handle, size, transpose, value);
}

inline void safe_glUniformMatrix2x3fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix2x3fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix3x2fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix3x2fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix2x4fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix2x4fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix4x2fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix4x2fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix3x4fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix3x4fv(handle, size, transpose, value);
}
inline void safe_glUniformMatrix4x3fv(const GLint handle, const GLsizei size, const GLboolean transpose, const GLfloat *value) {
   if (handle >= 0)
      glUniformMatrix4x3fv(handle, size, transpose, value);
}

#ifdef USE_SAFE_GL
#define glGetAttribLocation safe_glGetAttribLocation
#define glGetUniformLocation safe_glGetUniformLocation
#define glEnableVertexAttribArray safe_glEnableVertexAttribArray
#define glDisableVertexAttribArray safe_glDisableVertexAttribArray
#define glVertexAttribPointer safe_glVertexAttribPointer

#define glUniform1f safe_glUniform1f
#define glUniform2f safe_glUniform2f
#define glUniform3f safe_glUniform3f
#define glUniform4f safe_glUniform4f

#define glUniform1i safe_glUniform1i
#define glUniform2i safe_glUniform2i
#define glUniform3i safe_glUniform3i
#define glUniform4i safe_glUniform4i

// #define glUniform1ui safe_glUniform1ui
// #define glUniform2ui safe_glUniform2ui
// #define glUniform3ui safe_glUniform3ui
// #define glUniform4ui safe_glUniform4ui

#define glUniform1fv safe_glUniform1fv
#define glUniform2fv safe_glUniform2fv
#define glUniform3fv safe_glUniform3fv
#define glUniform4fv safe_glUniform4fv

#define glUniform1iv safe_glUniform1iv
#define glUniform2iv safe_glUniform2iv
#define glUniform3iv safe_glUniform3iv
#define glUniform4iv safe_glUniform4iv

// #define glUniform1uiv safe_glUniform1uiv
// #define glUniform2uiv safe_glUniform2uiv
// #define glUniform3uiv safe_glUniform3uiv
// #define glUniform4uiv safe_glUniform4uiv

#define glUniformMatrix2fv safe_glUniformMatrix2fv
#define glUniformMatrix3fv safe_glUniformMatrix3fv
#define glUniformMatrix4fv safe_glUniformMatrix4fv

#define glUniformMatrix2x3fv safe_glUniformMatrix2x3fv
#define glUniformMatrix3x2fv safe_glUniformMatrix3x2fv
#define glUniformMatrix2x4fv safe_glUniformMatrix2x4fv
#define glUniformMatrix4x2fv safe_glUniformMatrix4x2fv
#define glUniformMatrix3x4fv safe_glUniformMatrix3x4fv
#define glUniformMatrix4x3fv safe_glUniformMatrix4x3fv
#endif // USE_SAFE_GL

// Prevents opengl from switching shader programs if it doesn't need to
static int __current_shader_program__ = -1;
inline void smart_glUseProgram(unsigned int progHandle) {
   if (progHandle != __current_shader_program__) {
      glUseProgram(progHandle);
      __current_shader_program__ = progHandle;
   }
}

#ifdef USE_SMART_USE_PROGRAM
#define glUseProgram smart_glUseProgram
#endif // USE_SMART_USE_PROGRAM

#endif // __SAFE_GL__
